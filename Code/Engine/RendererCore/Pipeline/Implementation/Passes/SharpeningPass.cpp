#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/Passes/SharpeningPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/SharpeningConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSharpeningPass, 1, ezRTTIDefaultAllocator<ezSharpeningPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("Strength", m_fStrength)->AddAttributes(new ezDefaultValueAttribute(0.5f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezSharpeningPass::ezSharpeningPass()
  : ezRenderPipelinePass("SharpeningPass")
  , m_fStrength(0.5f)
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Load Shader
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Sharpening.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load Sharpening shader!");
  }

  // Load resources
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezSharpeningConstants>();
  }
}

ezSharpeningPass::~ezSharpeningPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezSharpeningPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Input
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

  {
    ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void ezSharpeningPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
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

    renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pColorInput->m_TextureHandle));

    ezGALUnorderedAccessViewHandle hOutput;
    {
      ezGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pColorOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }
    renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);

    renderViewContext.m_pRenderContext->BindConstantBuffer("ezSharpeningConstants", m_hConstantBuffer);

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

void ezSharpeningPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
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

void ezSharpeningPass::UpdateConstantBuffer() const
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezSharpeningConstants>(m_hConstantBuffer);
  constants->Strength = m_fStrength;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SharpeningPass);
