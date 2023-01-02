#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/ChromaticAberrationPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/ChromaticAberrationConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezChromaticAberrationPass, 1, ezRTTIDefaultAllocator<ezChromaticAberrationPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_ACCESSOR_PROPERTY("OffsetTexture", GetOffsetTextureFile, SetOffsetTextureFile)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
    EZ_MEMBER_PROPERTY("Strength", m_fStrength)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, {})),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezChromaticAberrationPass::ezChromaticAberrationPass()
  : ezRenderPipelinePass("ChromaticAberrationPass", true)
  , m_fStrength(1.0f)
{
  // Load shader.
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/ChromaticAberration.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Failed to load shader 'ChromaticAberration.ezShader'.");
  }

  // Load resources.
  {
    m_hOffsetTexture = ezResourceManager::LoadResource<ezTexture2DResource>("Black.color");
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezChromaticAberrationConstants>();
  }
}

ezChromaticAberrationPass::~ezChromaticAberrationPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezChromaticAberrationPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' input must allow shader resource view.", GetName());
      return false;
    }

    {
      ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
      desc.m_bAllowUAV = true;
      desc.m_bCreateRenderTarget = true;
      outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
    }
  }
  else
  {
    ezLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void ezChromaticAberrationPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* pColorInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* pColorOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pColorInput == nullptr || pColorOutput == nullptr)
  {
    return;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext);

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    const ezGALResourceViewHandle hInput = pDevice->GetDefaultResourceView(pColorInput->m_TextureHandle);
    ezGALUnorderedAccessViewHandle hOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pColorOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
    renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", hInput);
    renderViewContext.m_pRenderContext->BindTexture2D("OffsetTexture", m_hOffsetTexture, ezResourceAcquireMode::BlockTillLoaded);
    renderViewContext.m_pRenderContext->BindConstantBuffer("ezChromaticAberrationConstants", m_hConstantBuffer);

    const ezUInt32 uiWidth = pColorOutput->m_Desc.m_uiWidth;
    const ezUInt32 uiHeight = pColorOutput->m_Desc.m_uiHeight;

    const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    UpdateConstantBuffer();

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void ezChromaticAberrationPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  const ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  const ezGALTexture* pDest = pDevice->GetTexture(pOutput->m_TextureHandle);

  if (const ezGALTexture* pSource = pDevice->GetTexture(pInput->m_TextureHandle); pDest->GetDescription().m_Format != pSource->GetDescription().m_Format)
  {
    // TODO: use a shader when the format doesn't match exactly

    ezLog::Error("Copying textures of different formats is not implemented");
  }
  else
  {
    auto pCommandEncoder = ezRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

    pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
  }
}

void ezChromaticAberrationPass::UpdateConstantBuffer() const
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezChromaticAberrationConstants>(m_hConstantBuffer);
  constants->Strength = m_fStrength;
}

void ezChromaticAberrationPass::SetOffsetTextureFile(const char* szFile)
{
  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    m_hOffsetTexture = ezResourceManager::LoadResource<ezTexture2DResource>(szFile);
  }
}

const char* ezChromaticAberrationPass::GetOffsetTextureFile() const
{
  if (!m_hOffsetTexture.IsValid())
    return "";

  return m_hOffsetTexture.GetResourceID();
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_ChromaticAberrationPass);
