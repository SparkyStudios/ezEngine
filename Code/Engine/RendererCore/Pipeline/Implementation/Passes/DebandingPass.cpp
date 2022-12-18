#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/Passes/DebandingPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezDebandingPass, 1, ezRTTIDefaultAllocator<ezDebandingPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Color", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezDebandingPass::ezDebandingPass()
  : ezRenderPipelinePass("DebandingPass")
{
  // Loading shaders
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Debanding.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load Debanding shader!");
  }
}

ezDebandingPass::~ezDebandingPass() = default;

bool ezDebandingPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' color input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No color input connected to '{0}'!", GetName());
    return false;
  }

  {
    ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void ezDebandingPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
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

    renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));

    ezGALUnorderedAccessViewHandle hOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }
    renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);

    const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
    const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

    constexpr ezUInt32 uiThreadsX = 8;
    constexpr ezUInt32 uiThreadsY = 8;
    const ezUInt32 uiDispatchX = (uiWidth + uiThreadsX - 1) / uiThreadsX;
    const ezUInt32 uiDispatchY = (uiHeight + uiThreadsY - 1) / uiThreadsY;

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 6).IgnoreResult();
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void ezDebandingPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  auto pCommandEncoder = ezRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

  pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_DebandingPass);
