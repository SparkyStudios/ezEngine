#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/Passes/DepthOfFieldPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/DepthOfFieldConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezDepthOfFieldPass, 1, ezRTTIDefaultAllocator<ezDepthOfFieldPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Color", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("DepthStencil", m_PinDepth),
    EZ_MEMBER_PROPERTY("Radius", m_fRadius)->AddAttributes(new ezDefaultValueAttribute(5.5f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezDepthOfFieldPass::ezDepthOfFieldPass()
  : ezRenderPipelinePass("DepthOfFieldPass")
  , m_fRadius(5.5f)
{
  // Loading shaders
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/DepthOfField.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load DepthOfField shader!");
  }

  // Loading resources
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezDepthOfFieldConstants>();
  }
}

ezDepthOfFieldPass::~ezDepthOfFieldPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezDepthOfFieldPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
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
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void ezDepthOfFieldPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pDepth = inputs[m_PinDepth.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pDepth == nullptr || pOutput == nullptr)
  {
    return;
  }

  ezTempHashedString sCircleOfConfusionPass = "DOF_PASS_MODE_COC";
  ezTempHashedString sBokehPass = "DOF_PASS_MODE_BOKEH";
  ezTempHashedString sTentPass = "DOF_PASS_MODE_TENT";
  ezTempHashedString sBlendPass = "DOF_PASS_MODE_UPSCALE_BLEND";

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
  const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

  const ezUInt32 uiWidthHalf = uiWidth / 2;
  const ezUInt32 uiHeightHalf = uiHeight / 2;

  ezGALTextureHandle hBokehTexture1;
  ezGALTextureHandle hBokehTexture2;
  {
    ezGALTextureCreationDescription desc = pInput->m_Desc;
    desc.m_uiWidth = uiWidthHalf;
    desc.m_uiHeight = uiHeightHalf;
    desc.m_bAllowUAV = true;
    desc.m_bAllowShaderResourceView = true;

    hBokehTexture1 = pDevice->CreateTexture(desc);
    hBokehTexture2 = pDevice->CreateTexture(desc);
  }

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {
    // Circle of Confusion Pass
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Circle of Confusion");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      ezGALUnorderedAccessViewHandle hOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBokehTexture1;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", pDevice->GetDefaultResourceView(pDepth->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sCircleOfConfusionPass);

      const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      ezGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      ezVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = ezVec4(uiWidthHalf, uiHeightHalf, 1.0f / uiWidthHalf, 1.0f / uiHeightHalf);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }

    // Bokeh Pass
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Bokeh");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      ezGALUnorderedAccessViewHandle hOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBokehTexture2;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(hBokehTexture1));
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sBokehPass);

      const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      ezGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      ezVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = ezVec4(uiWidthHalf, uiHeightHalf, 1.0f / uiWidthHalf, 1.0f / uiHeightHalf);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }

    // Tent Pass
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Tent");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      ezGALUnorderedAccessViewHandle hOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBokehTexture1;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(hBokehTexture2));
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sTentPass);

      const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      ezGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      ezVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = ezVec4(uiWidthHalf, uiHeightHalf, 1.0f / uiWidthHalf, 1.0f / uiHeightHalf);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }

    // Upscale Blend Pass
    {
      auto pCommandEncoder = ezRenderContext::BeginComputeScope(pPass, renderViewContext, "Upscale Blend");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      ezGALUnorderedAccessViewHandle hOutput;
      {
        ezGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = pOutput->m_TextureHandle;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindTexture2D("BokehTexture", pDevice->GetDefaultResourceView(hBokehTexture1));
      renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", pDevice->GetDefaultResourceView(pDepth->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindConstantBuffer("ezDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sBlendPass);

      const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      ezGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      ezVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = ezVec4(uiWidth, uiHeight, 1.0f / uiWidth, 1.0f / uiHeight);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);

  // Cleanup resources
  pDevice->DestroyTexture(hBokehTexture1);
  pDevice->DestroyTexture(hBokehTexture2);
}

void ezDepthOfFieldPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
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

void ezDepthOfFieldPass::UpdateConstantBuffer()
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezDepthOfFieldConstants>(m_hConstantBuffer);
  constants->Radius = m_fRadius;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_DepthOfFieldPass);
