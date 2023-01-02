#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/FramePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/FrameConstants.h"

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezFramePass, 1, ezRTTIDefaultAllocator<ezFramePass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Input", m_PinInput),
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("Ratio", m_Ratio)->AddAttributes(new ezDefaultValueAttribute(ezVec2(16, 9))),
    EZ_MEMBER_PROPERTY("Color", m_Color)->AddAttributes(new ezDefaultValueAttribute(ezColor::Black))
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezFramePass::ezFramePass()
  : ezRenderPipelinePass("FramePass", true)
  , m_Ratio(ezVec2(16, 9))
{
  {
    // Load shader.
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/Frame.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Failed to load shader 'Frame.ezShader'.");
  }

  {
    m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezFrameConstants>();
  }
}

ezFramePass::~ezFramePass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool ezFramePass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  const ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  const ezGALRenderTargets& renderTargets = view.GetActiveRenderTargets();

  // Color
  if (const auto* pColorInput = inputs[m_PinInput.m_uiInputIndex]; pColorInput != nullptr)
  {
    if (!pColorInput->m_bAllowShaderResourceView)
    {
      ezLog::Error("'{0}' input must allow shader resource view.", GetName());
      return false;
    }

    if (const ezGALTexture* pTexture = pDevice->GetTexture(renderTargets.m_hRTs[0]))
    {
      const ezGALTextureCreationDescription& desc = pTexture->GetDescription();

      outputs[m_PinOutput.m_uiOutputIndex].SetAsRenderTarget(pColorInput->m_uiWidth, pColorInput->m_uiHeight, desc.m_Format);
      outputs[m_PinOutput.m_uiOutputIndex].m_uiArraySize = pColorInput->m_uiArraySize;
    }

    // outputs[m_PinBloomOutput.m_uiOutputIndex] = *inputs[m_PinInput.m_uiInputIndex];
  }
  else
  {
    ezLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void ezFramePass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  const ezUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
  const ezUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {
    const ezGALTextureHandle& hInput = pInput->m_TextureHandle;
    const ezGALTextureHandle& hOutput = pOutput->m_TextureHandle;

    // Setup render target
    ezGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(hOutput));

    // Bind render target and viewport
    // auto pCommandEncoder = ezRenderContext::BeginPassAndRenderingScope(renderViewContext, renderingSetup, GetName(), renderViewContext.m_pCamera->IsStereoscopic());
    renderViewContext.m_pRenderContext->BeginRendering(pPass, renderingSetup, ezRectFloat(uiWidth, uiHeight), "", renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->BindShader(m_hShader);
    renderViewContext.m_pRenderContext->BindConstantBuffer("ezFrameConstants", m_hConstantBuffer);

    renderViewContext.m_pRenderContext->BindMeshBuffer(ezGALBufferHandle(), ezGALBufferHandle(), nullptr, ezGALPrimitiveTopology::Triangles, 1);

    UpdateConstantBuffer();

    renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(hInput));
    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

    renderViewContext.m_pRenderContext->EndRendering();
  }
  pDevice->EndPass(pPass);
}

void ezFramePass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
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

void ezFramePass::UpdateConstantBuffer() const
{
  auto* constants = ezRenderContext::GetConstantBufferData<ezFrameConstants>(m_hConstantBuffer);
  constants->Ratio = m_Ratio.x / m_Ratio.y;
  constants->Color = m_Color.GetAsVec4().GetAsVec3();
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_FramePass);
