#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/UpscalePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

#define A_CPU
#include "../../../../Data/Base/Shaders/ThirdParty/FFX/ffx_a.h"
#include "../../../../Data/Base/Shaders/ThirdParty/FFX/ffx_fsr1.h"

#include "../../../../Data/Base/Shaders/Pipeline/UpscaleConstants.h"

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezUpscaleMode, 1)
  EZ_ENUM_CONSTANT(ezUpscaleMode::FSR),
  EZ_ENUM_CONSTANT(ezUpscaleMode::BiLinear)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezFSRUpscalePreset, 1)
  EZ_ENUM_CONSTANT(ezFSRUpscalePreset::UltraQuality),
  EZ_ENUM_CONSTANT(ezFSRUpscalePreset::Quality),
  EZ_ENUM_CONSTANT(ezFSRUpscalePreset::Balanced),
  EZ_ENUM_CONSTANT(ezFSRUpscalePreset::Performance),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezUpscalePass, 1, ezRTTIDefaultAllocator<ezUpscalePass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_ENUM_MEMBER_PROPERTY("Mode", ezUpscaleMode, m_eUpscaleMode),
    EZ_ENUM_MEMBER_PROPERTY("FSR_Preset", ezFSRUpscalePreset, m_eFSRPreset),
    EZ_MEMBER_PROPERTY("FSR_Sharpen", m_bFSRSharpen),
    EZ_MEMBER_PROPERTY("FSR_Sharpness", m_fFSRSharpness)->AddAttributes(new ezDefaultValueAttribute(0.87f), new ezClampValueAttribute(0.0f, 1.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


ezUpscalePass::ezUpscalePass()
  : ezRenderPipelinePass("UpscalePass", false)
  , m_eUpscaleMode(ezUpscaleMode::FSR)
  , m_eFSRPreset(ezFSRUpscalePreset::UltraQuality)
  , m_bFSRSharpen(true)
  , m_fFSRSharpness(0.87f)
{
  // Load shader
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Upscale.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load Luminance Pass shader!");
  }

  // Load resources
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezUpscaleConstants>();
  }
}

ezUpscalePass::~ezUpscalePass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezUpscalePass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' Color input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No Color input connected to '{0}'!", GetName());
    return false;
  }

  // Output has the window resolution
  {
    ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_uiWidth = static_cast<ezUInt32>(view.GetTargetViewport().width);
    desc.m_uiHeight = static_cast<ezUInt32>(view.GetTargetViewport().height);
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void ezUpscalePass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  // AMD FidelityFX Super Resolution 1.0 (FSR1).
  // Provides high-quality solution designed to produce high resolution frames from lower resolution inputs.
  // GitHub:        https://github.com/GPUOpen-Effects/FidelityFX-FSR
  // Documentation: https://github.com/GPUOpen-Effects/FidelityFX-FSR/blob/master/docs/FidelityFX-FSR-Overview-Integration.pdf

  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pInput == nullptr || pOutput == nullptr)
    return;

  // Early exit - Nothing to upscale if render resolution = target resolution
  if (renderViewContext.m_pViewData->m_ViewPortRect == renderViewContext.m_pViewData->m_TargetViewportRect)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
  const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

  // As per documentation - Page 23
  static constexpr int threadGroupWorkRegionDim = 16;
  const ezUInt32 uiDispatchX = (uiWidth + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
  const ezUInt32 uiDispatchY = (uiHeight + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {
    AU1 const0[4];
    AU1 const1[4];
    AU1 const2[4];
    AU1 const3[4];

    FsrEasuCon(
      const0, const1, const2, const3, // FSR constants
      static_cast<AF1>(renderViewContext.m_pViewData->m_ViewPortRect.width),
      static_cast<AF1>(renderViewContext.m_pViewData->m_ViewPortRect.height), // Render resolution
      static_cast<AF1>(renderViewContext.m_pViewData->m_ViewPortRect.width),
      static_cast<AF1>(renderViewContext.m_pViewData->m_ViewPortRect.height), // Input Texture resolution, same as render resolution in our case
      static_cast<AF1>(renderViewContext.m_pViewData->m_TargetViewportRect.width),
      static_cast<AF1>(renderViewContext.m_pViewData->m_TargetViewportRect.height) // Target (window) resolution
    );

    ezGALUnorderedAccessViewHandle hOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    ezGALSamplerStateHandle hSamplerState;
    {
      ezGALSamplerStateCreationDescription desc;
      desc.m_MinFilter = ezGALTextureFilterMode::Linear;
      desc.m_MagFilter = ezGALTextureFilterMode::Linear;
      desc.m_MipFilter = ezGALTextureFilterMode::Point;
      desc.m_AddressU = ezImageAddressMode::Clamp;
      desc.m_AddressV = ezImageAddressMode::Clamp;
      desc.m_AddressW = ezImageAddressMode::Clamp;
      desc.m_uiMaxAnisotropy = 1;
      desc.m_fMipLodBias = -ezMath::Log2(static_cast<float>(pOutput->m_Desc.m_uiWidth) / static_cast<float>(pInput->m_Desc.m_uiWidth));
      hSamplerState = pDevice->CreateSamplerState(desc);
    }

    if (m_eUpscaleMode == ezUpscaleMode::FSR)
    {
      ezGALTextureHandle hIntermediaryTexture;
      ezGALUnorderedAccessViewHandle hIntermediaryOutput;

      if (m_bFSRSharpen)
      {
        {
          ezGALTextureCreationDescription desc;
          desc.m_uiHeight = uiHeight;
          desc.m_uiWidth = uiWidth;
          desc.m_Format = pOutput->m_Desc.m_Format;
          desc.m_bAllowShaderResourceView = true;
          desc.m_bAllowUAV = true;
          desc.m_Type = ezGALTextureType::Texture2D;
          desc.m_ResourceAccess.m_bImmutable = false;
          hIntermediaryTexture = pDevice->CreateTexture(desc);
        }

        {
          ezGALUnorderedAccessViewCreationDescription desc;
          desc.m_hTexture = hIntermediaryTexture;
          desc.m_uiMipLevelToUse = 0;
          hIntermediaryOutput = pDevice->CreateUnorderedAccessView(desc);
        }
      }

      // Upscale Pass
      {
        auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "FSR Upscale");

        renderViewContext.m_pRenderContext->BindShader(m_hShader);

        renderViewContext.m_pRenderContext->BindUAV("Output", m_bFSRSharpen ? hIntermediaryOutput : hOutput);
        renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
        renderViewContext.m_pRenderContext->BindSamplerState("UpscaleSampler", hSamplerState);
        renderViewContext.m_pRenderContext->BindConstantBuffer("ezUpscaleConstants", m_hConstantBuffer);

        renderViewContext.m_pRenderContext->SetShaderPermutationVariable("UPSCALE_MODE", "UPSCALE_MODE_FSR_UPSCALE");

        auto* constants = ezRenderContext::GetConstantBufferData<ezUpscaleConstants>(m_hConstantBuffer);
        ezMemoryUtils::Copy(constants->Const0.GetData(), &const0[0], 4);
        ezMemoryUtils::Copy(constants->Const1.GetData(), &const1[0], 4);
        ezMemoryUtils::Copy(constants->Const2.GetData(), &const2[0], 4);
        ezMemoryUtils::Copy(constants->Const3.GetData(), &const3[0], 4);

        renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      }

      if (m_bFSRSharpen)
      // Sharpen Pass
      {
        FsrRcasCon(const0, ezMath::Pow(0.5f, m_fFSRSharpness));

        auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "FSR Sharpen");

        renderViewContext.m_pRenderContext->BindShader(m_hShader);

        renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
        renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(hIntermediaryTexture));
        renderViewContext.m_pRenderContext->BindSamplerState("UpscaleSampler", hSamplerState);
        renderViewContext.m_pRenderContext->BindConstantBuffer("ezUpscaleConstants", m_hConstantBuffer);

        renderViewContext.m_pRenderContext->SetShaderPermutationVariable("UPSCALE_MODE", "UPSCALE_MODE_FSR_SHARPEN");

        auto* constants = ezRenderContext::GetConstantBufferData<ezUpscaleConstants>(m_hConstantBuffer);
        ezMemoryUtils::Copy(constants->Const0.GetData(), &const0[0], 4);

        renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();

        pDevice->DestroyTexture(hIntermediaryTexture);
      }
    }
    else
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "BiLinear Upscale");

      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindSamplerState("UpscaleSampler", hSamplerState);
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezUpscaleConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("UPSCALE_MODE", "UPSCALE_MODE_BILINEAR");

      auto* constants = ezRenderContext::GetConstantBufferData<ezUpscaleConstants>(m_hConstantBuffer);
      ezMemoryUtils::Copy(constants->Const0.GetData(), &const0[0], 4);
      ezMemoryUtils::Copy(constants->Const1.GetData(), &const1[0], 4);
      ezMemoryUtils::Copy(constants->Const2.GetData(), &const2[0], 4);
      ezMemoryUtils::Copy(constants->Const3.GetData(), &const3[0], 4);

      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
    }

    // Cleanup
    pDevice->DestroySamplerState(hSamplerState);
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void ezUpscalePass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  auto pCommandEncoder = ezRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

  pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_UpscalePass);
