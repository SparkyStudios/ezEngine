#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/Passes/MotionBlurPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include "../../../../Data/Base/Shaders/Pipeline/MotionBlurConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMotionBlurPass, 1, ezRTTIDefaultAllocator<ezMotionBlurPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Color", m_PinInputColor),
    EZ_MEMBER_PROPERTY("Velocity", m_PinInputVelocity),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("Strength", m_fStrength)->AddAttributes(new ezClampValueAttribute(0.0f, 1.0f), new ezDefaultValueAttribute(1.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezMotionBlurPass::ezMotionBlurPass()
  : ezRenderPipelinePass("MotionBlurPass")
  , m_fStrength(1.0f)
{
  // Load shaders
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/MotionBlur.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load MotionBlur shader!");
  }

  // Load resources
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezMotionBlurConstants>();
  }
}

ezMotionBlurPass::~ezMotionBlurPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezMotionBlurPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Velocity
  if (inputs[m_PinInputVelocity.m_uiInputIndex])
  {
    if (!inputs[m_PinInputVelocity.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' velocity input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    ezLog::Error("No velocity input connected to '{0}'!", GetName());
    return false;
  }

  // Color
  if (inputs[m_PinInputColor.m_uiInputIndex])
  {
    if (!inputs[m_PinInputColor.m_uiInputIndex]->m_bAllowShaderResourceView)
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
    ezGALTextureCreationDescription desc = *inputs[m_PinInputColor.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void ezMotionBlurPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* pInputColor = inputs[m_PinInputColor.m_uiInputIndex];
  const auto* pInputVelocity = inputs[m_PinInputVelocity.m_uiInputIndex];
  if (pInputColor == nullptr || pInputVelocity == nullptr)
  {
    return;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  ezGALPass* pGALPass = pDevice->BeginPass(GetName());
  EZ_SCOPE_EXIT(
    pDevice->EndPass(pGALPass);
    renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading));

  if (const auto* pColorOutput = outputs[m_PinOutput.m_uiOutputIndex]; pColorOutput != nullptr && !pColorOutput->m_TextureHandle.IsInvalidated())
  {
    auto pCommandEncoder = ezRenderContext::BeginComputeScope(pGALPass, renderViewContext);
    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(pInputColor->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindTexture2D("VelocityTexture", pDevice->GetDefaultResourceView(pInputVelocity->m_TextureHandle));

    ezGALUnorderedAccessViewHandle hMotionBlurOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pColorOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hMotionBlurOutput = pDevice->CreateUnorderedAccessView(desc);
    }
    renderViewContext.m_pRenderContext->BindUAV("MotionBlurOutput", hMotionBlurOutput);

    const ezUInt32 uiWidth = pColorOutput->m_Desc.m_uiWidth;
    const ezUInt32 uiHeight = pColorOutput->m_Desc.m_uiHeight;

    const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    auto* constants = ezRenderContext::GetConstantBufferData<ezMotionBlurConstants>(m_hConstantBuffer);
    constants->MotionBlurStrength = m_fStrength;

    renderViewContext.m_pRenderContext->BindConstantBuffer("ezMotionBlurConstants", m_hConstantBuffer);

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 6).IgnoreResult();
  }
}

void ezMotionBlurPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto pInput = inputs[m_PinInputColor.m_uiInputIndex];
  const auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  auto pCommandEncoder = ezRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

  pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_MotionBlurPass);
