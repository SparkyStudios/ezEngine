#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/TonemapPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/TonemapConstants.h"

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezTonemapMode, 1)
  EZ_ENUM_CONSTANT(ezTonemapMode::AMD),
  EZ_ENUM_CONSTANT(ezTonemapMode::ACES),
  EZ_ENUM_CONSTANT(ezTonemapMode::Uncharted2),
  EZ_ENUM_CONSTANT(ezTonemapMode::Reinhard),
  EZ_ENUM_CONSTANT(ezTonemapMode::None),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTonemapPass, 2, ezRTTIDefaultAllocator<ezTonemapPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_ENUM_MEMBER_PROPERTY("Mode", ezTonemapMode, m_eTonemapMode)->AddAttributes(new ezDefaultValueAttribute(ezTonemapMode::AMD)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezTonemapPass::ezTonemapPass()
  : ezRenderPipelinePass("TonemapPass", true)
  , m_eTonemapMode(ezTonemapMode::AMD)
{
  // Loading shaders
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Tonemap.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load Tonemap shader.");
  }

  // Loading resources
  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezTonemapConstants>();
  }
}

ezTonemapPass::~ezTonemapPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezTonemapPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
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
    ezLog::Error("No input connected to '{0}'.", GetName());
    return false;
  }

  ezGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
  desc.m_bAllowUAV = true;
  outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);

  return true;
}

void ezTonemapPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
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

    renderViewContext.m_pRenderContext->BindConstantBuffer("ezTonemapConstants", m_hConstantBuffer);

    const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
    const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

    const ezUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const ezUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    UpdateConstantBuffer();

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 6).IgnoreResult();
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void ezTonemapPass::UpdateConstantBuffer()
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezTonemapConstants>(m_hConstantBuffer);
  constants->ToneMappingMode = m_eTonemapMode.GetValue();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class ezTonemapPassPatch_1_2 : public ezGraphPatch
{
public:
  ezTonemapPassPatch_1_2()
    : ezGraphPatch("ezTonemapPass", 2)
  {
  }

  virtual void Patch(ezGraphPatchContext& context, ezAbstractObjectGraph* pGraph, ezAbstractObjectNode* pNode) const override
  {
    pNode->AddProperty("Mode", ezTonemapMode::AMD);
    pNode->RenameProperty("Color", "Input");
    pNode->RemoveProperty("Bloom");
  }
};

ezTonemapPassPatch_1_2 g_ezTonemapPassPatch_1_2;

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_TonemapPass);
