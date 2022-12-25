#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/BloomPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/BloomConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezBloomPass, 1, ezRTTIDefaultAllocator<ezBloomPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("Intensity", m_fIntensity)->AddAttributes(new ezDefaultValueAttribute(0.3f)),
    EZ_MEMBER_PROPERTY("MipCount", m_uiMipCount)->AddAttributes(new ezClampValueAttribute(1, 12), new ezDefaultValueAttribute(6)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezBloomPass::ezBloomPass()
  : ezRenderPipelinePass("BloomPass", true)
  , m_fIntensity(0.3f)
  , m_uiMipCount(6)
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Load shader.
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Bloom.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load bloom shader!");
  }

  // Load resources.
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezBloomConstants>();

    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = 4;
    desc.m_uiTotalSize = desc.m_uiStructSize;
    desc.m_BufferType = ezGALBufferType::Generic;
    desc.m_bAllowUAV = true;
    desc.m_bUseAsStructuredBuffer = true;
    desc.m_bAllowShaderResourceView = true;
    desc.m_ResourceAccess.m_bImmutable = false;

    m_DownsampleAtomicCounter = EZ_NEW_ARRAY(ezAlignedAllocatorWrapper::GetAllocator(), ezAtomicCounterBuffer, 1);
    m_DownsampleAtomicCounter[0].Value = 0;

    m_hDownsampleAtomicCounterBuffer = pDevice->CreateBuffer(desc, m_DownsampleAtomicCounter.ToByteArray());
  }
}

ezBloomPass::~ezBloomPass()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  pDevice->DestroyBuffer(m_hDownsampleAtomicCounterBuffer);

  EZ_DELETE_ARRAY(ezAlignedAllocatorWrapper::GetAllocator(), m_DownsampleAtomicCounter);

  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezBloomPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  // Bloom Output
  {
    ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bAllowShaderResourceView = true;

    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void ezBloomPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  ezTempHashedString sLuminancePass = "BLOOM_PASS_MODE_LUMINANCE";
  ezTempHashedString sUpscaleBlendPass = "BLOOM_PASS_MODE_UPSCALE_BLEND_MIP";
  ezTempHashedString sColorBlendPass = "BLOOM_PASS_MODE_BLEND_FRAME";

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  EZ_SCOPE_EXIT(
    pDevice->EndPass(pPass);
    renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading););

  const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
  const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

  ezGALTextureHandle hBloomTexture;
  {
    ezGALTextureCreationDescription desc = pInput->m_Desc;
    desc.m_uiMipLevelCount = m_uiMipCount;
    desc.m_bAllowUAV = true;
    desc.m_bAllowShaderResourceView = true;
    desc.m_Format = ezGALResourceFormat::RG11B10Float;
    hBloomTexture = pDevice->CreateTexture(desc);
  }

  // Luminance pass
  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Luminance");

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    ezGALUnorderedAccessViewHandle hLuminanceOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = hBloomTexture;
      desc.m_uiMipLevelToUse = 0;
      hLuminanceOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("Output", hLuminanceOutput);
    renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindConstantBuffer("ezBloomConstants", m_hConstantBuffer);

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", sLuminancePass);

    const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    UpdateBloomConstantBuffer(ezVec2(1.0f / static_cast<float>(uiWidth), 1.0f / static_cast<float>(uiHeight)), uiDispatchX * uiDispatchY);

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();

    // Cleanup
    pCommandEncoder->UnsetResourceViews(pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
    pCommandEncoder->UnsetUnorderedAccessViews(hLuminanceOutput);
  }

  // Downsample pass
  {
    DownsamplePass(pPass, renderViewContext, hBloomTexture, uiWidth, uiHeight);
  }

  // Upsample and Blend pass
  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "UpscaleBlend");

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    for (ezUInt32 i = m_uiMipCount - 1; i > 0; --i)
    {
      ezUInt32 uiMipSmall = i;
      ezUInt32 uiMipLarge = i - 1;

      ezUInt32 uiMipLargeWidth = uiWidth >> uiMipLarge;
      ezUInt32 uiMipLargeHeight = uiHeight >> uiMipLarge;

      ezGALResourceViewHandle hUpscaleBlendInput;
      {
        ezGALResourceViewCreationDescription desc;
        desc.m_hTexture = hBloomTexture;
        desc.m_uiMostDetailedMipLevel = uiMipSmall;
        desc.m_uiMipLevelsToUse = 1;
        desc.m_bUnsetUAV = false;
        hUpscaleBlendInput = pDevice->CreateResourceView(desc);
      }

      ezGALUnorderedAccessViewHandle hUpscaleBlendOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBloomTexture;
        desc.m_uiMipLevelToUse = uiMipLarge;
        desc.m_bUnsetResourceView = false;
        hUpscaleBlendOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hUpscaleBlendOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", hUpscaleBlendInput);
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezBloomConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", sUpscaleBlendPass);

      const auto uiDispatchX = static_cast<ezUInt32>(ezMath::Ceil(static_cast<float>(uiMipLargeWidth) / THREAD_GROUP_COUNT_X));
      const auto uiDispatchY = static_cast<ezUInt32>(ezMath::Ceil(static_cast<float>(uiMipLargeHeight) / THREAD_GROUP_COUNT_Y));

      UpdateBloomConstantBuffer(ezVec2(1.0f / static_cast<float>(uiMipLargeWidth), 1.0f / static_cast<float>(uiMipLargeHeight)), uiDispatchX * uiDispatchY);

      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();

      // Cleanup
      pCommandEncoder->UnsetResourceViews(hUpscaleBlendInput);
      pCommandEncoder->UnsetUnorderedAccessViews(hUpscaleBlendOutput);
    }
  }

  // Color Blend
  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "ColorBlend");

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    ezGALResourceViewHandle hBloomInput;
    {
      ezGALResourceViewCreationDescription desc;
      desc.m_hTexture = hBloomTexture;
      desc.m_uiMostDetailedMipLevel = 0;
      desc.m_uiMipLevelsToUse = 1;
      hBloomInput = pDevice->CreateResourceView(desc);
    }

    ezGALUnorderedAccessViewHandle hColorBlendOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hColorBlendOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("Output", hColorBlendOutput);
    renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindTexture2D("MipTexture", hBloomInput);
    renderViewContext.m_pRenderContext->BindConstantBuffer("ezBloomConstants", m_hConstantBuffer);

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", sColorBlendPass);

    const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    UpdateBloomConstantBuffer(ezVec2(1.0f / static_cast<float>(uiWidth), 1.0f / static_cast<float>(uiHeight)), uiDispatchX * uiDispatchY);

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();

    // Cleanup
    pCommandEncoder->UnsetResourceViews(pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
    pCommandEncoder->UnsetResourceViews(hBloomInput);
    pCommandEncoder->UnsetUnorderedAccessViews(hColorBlendOutput);
  }

  // Cleanup resources
  pDevice->DestroyTexture(hBloomTexture);
}

void ezBloomPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pColorOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pColorOutput == nullptr)
  {
    return;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALRenderingSetup renderingSetup;
  renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pColorOutput->m_TextureHandle));
  renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;
  renderingSetup.m_ClearColor = ezColor::Black;

  auto pCommandEncoder = ezRenderContext::BeginPassAndRenderingScope(renderViewContext, renderingSetup, "Clear");
}

void ezBloomPass::UpdateBloomConstantBuffer(ezVec2 pixelSize, ezUInt32 uiWorkGroupCount)
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezBloomConstants>(m_hConstantBuffer);
  constants->PixelSize = pixelSize;
  constants->BloomIntensity = m_fIntensity;
  constants->MipCount = m_uiMipCount;
  constants->WorkGroupCount = uiWorkGroupCount;
}

void ezBloomPass::DownsamplePass(ezGALPass* pPass, const ezRenderViewContext& renderViewContext, const ezGALTextureHandle& hBloomTexture, ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  // AMD FidelityFX Single Pass Downsampler.
  // Provides an RDNA™-optimized solution for generating up to 12 MIP levels of a texture.
  // GitHub:        https://github.com/GPUOpen-Effects/FidelityFX-SPD
  // Documentation: https://github.com/GPUOpen-Effects/FidelityFX-SPD/blob/master/docs/FidelityFX_SPD.pdf

  if (hBloomTexture.IsInvalidated())
    return;

  const ezUInt32 uiOutputMipCount = m_uiMipCount - 1;
  const ezUInt32 uiSmallestWidth = uiWidth >> uiOutputMipCount;
  const ezUInt32 uiSmallestHeight = uiWidth >> uiOutputMipCount;

  // Ensure that the input texture meets the requirements.
  EZ_ASSERT_DEV(uiOutputMipCount + 1 <= 12, "AMD FidelityFX Single Pass Downsampler can't generate more than 12 mipmap levels."); // As per documentation (page 22)

  ezTempHashedString sDownscalePass = "BLOOM_PASS_MODE_DOWNSCALE";

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Downsample");

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    renderViewContext.m_pRenderContext->BindConstantBuffer("ezBloomConstants", m_hConstantBuffer);

    ezGALResourceViewHandle hBloomInput;
    {
      ezGALResourceViewCreationDescription desc;
      desc.m_hTexture = hBloomTexture;
      desc.m_uiMostDetailedMipLevel = 0;
      desc.m_uiMipLevelsToUse = 1;
      desc.m_bUnsetUAV = false;
      hBloomInput = pDevice->CreateResourceView(desc);
    }

    renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", hBloomInput);

    ezGALUnorderedAccessViewHandle hAtomicCounter;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_OverrideViewFormat = ezGALResourceFormat::RUInt;
      desc.m_hBuffer = m_hDownsampleAtomicCounterBuffer;
      desc.m_uiNumElements = 1;
      desc.m_uiFirstElement = 0;
      hAtomicCounter = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("AtomicCounter", hAtomicCounter);

    for (ezUInt32 i = 0; i < uiOutputMipCount; ++i)
    {
      ezStringBuilder sSlotName;
      sSlotName.Format("DownsampleOutput[{}]", i);

      ezGALUnorderedAccessViewHandle hDownsampleOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBloomTexture;
        desc.m_uiMipLevelToUse = i + 1;
        desc.m_bUnsetResourceView = false;
        hDownsampleOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV(sSlotName.GetView(), hDownsampleOutput);
    }

    // As per documentation (page 22)
    const ezUInt32 uiDispatchX = (uiWidth + 63) >> 6;
    const ezUInt32 uiDispatchY = (uiHeight + 63) >> 6;

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", sDownscalePass);

    UpdateBloomConstantBuffer(ezVec2(1.0f / static_cast<float>(uiWidth), 1.0f / static_cast<float>(uiHeight)), uiDispatchX * uiDispatchY);

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();

    // Clean up
    pCommandEncoder->UnsetUnorderedAccessViews(pDevice->GetTexture(hBloomTexture));
    pCommandEncoder->UnsetResourceViews(hBloomInput);
  }
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_BloomPass);
