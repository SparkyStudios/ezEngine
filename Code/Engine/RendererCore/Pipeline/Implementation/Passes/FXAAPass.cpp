#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/FXAAPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/FXAAConstants.h"

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezEdgeThresholdQuality, 1)
  EZ_ENUM_CONSTANT(ezEdgeThresholdQuality::Little),
  EZ_ENUM_CONSTANT(ezEdgeThresholdQuality::LowQuality),
  EZ_ENUM_CONSTANT(ezEdgeThresholdQuality::DefaultQuality),
  EZ_ENUM_CONSTANT(ezEdgeThresholdQuality::HighQuality),
  EZ_ENUM_CONSTANT(ezEdgeThresholdQuality::Overkill),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezEdgeThresholdMinQuality, 1)
  EZ_ENUM_CONSTANT(ezEdgeThresholdMinQuality::UpperLimit),
  EZ_ENUM_CONSTANT(ezEdgeThresholdMinQuality::HighQuality),
  EZ_ENUM_CONSTANT(ezEdgeThresholdMinQuality::VisibleLimit),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezFXAAPass, 1, ezRTTIDefaultAllocator<ezFXAAPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("SubPixelAliasingRemovalAmount", m_fSPARAmount)->AddAttributes(new ezDefaultValueAttribute(0.75f), new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_ENUM_MEMBER_PROPERTY("EdgeThreshold", ezEdgeThresholdQuality, m_eEdgeThreshold)->AddAttributes(new ezDefaultValueAttribute(ezEdgeThresholdQuality::Default)),
    EZ_ENUM_MEMBER_PROPERTY("EdgeThresholdMin", ezEdgeThresholdMinQuality, m_eEdgeThresholdMin)->AddAttributes(new ezDefaultValueAttribute(ezEdgeThresholdMinQuality::Default)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


ezFXAAPass::ezFXAAPass()
  : ezRenderPipelinePass("FXAAPass", true)
  , m_fSPARAmount(0.75f)
  , m_eEdgeThreshold(ezEdgeThresholdQuality::Default)
  , m_eEdgeThresholdMin(ezEdgeThresholdMinQuality::Default)
{
  // Load shader.
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/FXAA.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load FXAA Pass shader!");
  }

  // Init constant buffer
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezFXAAConstants>();
  }
}

ezFXAAPass::~ezFXAAPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezFXAAPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' Color input must allow shader resource view.", GetName());
      return false;
    }

    ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }
  else
  {
    ezLog::Error("No Color input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void ezFXAAPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pColorInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pColorOutput = outputs[m_PinOutput.m_uiOutputIndex];

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

    ezGALUnorderedAccessViewHandle hOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pColorOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
    renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pColorInput->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindConstantBuffer("ezFXAAConstants", m_hConstantBuffer);

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

void ezFXAAPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

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

void ezFXAAPass::UpdateConstantBuffer() const
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezFXAAConstants>(m_hConstantBuffer);
  constants->SubPixelAliasingRemovalAmount = m_fSPARAmount;

  switch (m_eEdgeThreshold)
  {
    case ezEdgeThresholdQuality::Little: constants->EdgeThreshold = 0.333f; break;
    case ezEdgeThresholdQuality::LowQuality: constants->EdgeThreshold = 0.250f; break;
    case ezEdgeThresholdQuality::DefaultQuality: constants->EdgeThreshold = 0.166f; break;
    case ezEdgeThresholdQuality::HighQuality: constants->EdgeThreshold = 0.125f; break;
    case ezEdgeThresholdQuality::Overkill: constants->EdgeThreshold = 0.063f; break;
  }

  switch (m_eEdgeThresholdMin)
  {
    case ezEdgeThresholdMinQuality::UpperLimit: constants->EdgeThresholdMin = 0.0833f; break;
    case ezEdgeThresholdMinQuality::HighQuality: constants->EdgeThresholdMin = 0.0625f; break;
    case ezEdgeThresholdMinQuality::VisibleLimit: constants->EdgeThresholdMin = 0.0312f; break;
  }
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_FXAAPass);
