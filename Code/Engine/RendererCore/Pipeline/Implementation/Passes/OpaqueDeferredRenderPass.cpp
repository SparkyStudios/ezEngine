#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/OpaqueDeferredRenderPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezOpaqueDeferredRenderPass, 1, ezRTTIDefaultAllocator<ezOpaqueDeferredRenderPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Output", m_PinOutput),
    EZ_MEMBER_PROPERTY("Albedo", m_PinAlbedo),
    EZ_MEMBER_PROPERTY("Normal", m_PinNormal),
    EZ_MEMBER_PROPERTY("Material", m_PinMaterial),
    EZ_MEMBER_PROPERTY("Velocity", m_PinVelocity),
    EZ_MEMBER_PROPERTY("SSAO", m_PinSSAO),
    EZ_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
    EZ_MEMBER_PROPERTY("WriteDepth", m_bWriteDepth),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezOpaqueDeferredRenderPass::ezOpaqueDeferredRenderPass(const char* szName)
  : ezRenderPipelinePass(szName, true)
  , m_bWriteDepth(false)
{
  // Load shaders
  {
    m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/DeferredLightning.ezShader");
    EZ_ASSERT_DEV(m_hShader.IsValid(), "Could not load DeferredLightning shader!");
  }
}

ezOpaqueDeferredRenderPass::~ezOpaqueDeferredRenderPass() = default;

bool ezOpaqueDeferredRenderPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // Albedo
  if (inputs[m_PinAlbedo.m_uiInputIndex])
  {
    outputs[m_PinAlbedo.m_uiOutputIndex] = *inputs[m_PinAlbedo.m_uiInputIndex];

    ezGALTextureCreationDescription desc = *inputs[m_PinAlbedo.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_Format = ezGALResourceFormat::RGBAHalf;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }
  else
  {
    ezLog::Error("No albedo input connected to pass '{0}'.", GetName());
    return false;
  }

  // Normal
  if (inputs[m_PinNormal.m_uiInputIndex])
  {
    outputs[m_PinNormal.m_uiOutputIndex] = *inputs[m_PinNormal.m_uiInputIndex];
  }
  else
  {
    ezLog::Error("No normal input connected to pass '{0}'.", GetName());
    return false;
  }

  // Material
  if (inputs[m_PinMaterial.m_uiInputIndex])
  {
    outputs[m_PinMaterial.m_uiOutputIndex] = *inputs[m_PinMaterial.m_uiInputIndex];
  }
  else
  {
    ezLog::Error("No material input connected to pass '{0}'.", GetName());
    return false;
  }

  // Velocity
  if (inputs[m_PinVelocity.m_uiInputIndex])
  {
    outputs[m_PinVelocity.m_uiOutputIndex] = *inputs[m_PinVelocity.m_uiInputIndex];
  }

  // Depth - Stencil
  if (inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    outputs[m_PinDepthStencil.m_uiOutputIndex] = *inputs[m_PinDepthStencil.m_uiInputIndex];
  }
  else
  {
    ezLog::Error("No depth/stencil input connected to pass '{0}'.", GetName());
    return false;
  }

  return true;
}

void ezOpaqueDeferredRenderPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALPass* pPass = pDevice->BeginPass(GetName());
  {
    // Rendering Pass
    {
      // Setup render targets
      ezGALRenderingSetup renderingSetup;

      if (inputs[m_PinAlbedo.m_uiInputIndex])
      {
        renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(inputs[m_PinAlbedo.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinVelocity.m_uiInputIndex])
      {
        renderingSetup.m_RenderTargetSetup.SetRenderTarget(1, pDevice->GetDefaultRenderTargetView(inputs[m_PinVelocity.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinNormal.m_uiInputIndex])
      {
        renderingSetup.m_RenderTargetSetup.SetRenderTarget(2, pDevice->GetDefaultRenderTargetView(inputs[m_PinNormal.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinMaterial.m_uiInputIndex])
      {
        renderingSetup.m_RenderTargetSetup.SetRenderTarget(3, pDevice->GetDefaultRenderTargetView(inputs[m_PinMaterial.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinDepthStencil.m_uiInputIndex])
      {
        renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(inputs[m_PinDepthStencil.m_uiInputIndex]->m_TextureHandle));
      }

      renderViewContext.m_pRenderContext->BeginRendering(pPass, renderingSetup, renderViewContext.m_pViewData->m_ViewPortRect, "Rendering", renderViewContext.m_pCamera->IsStereoscopic());

      SetupPermutationVars(renderViewContext);

      RenderObjects(renderViewContext);

      renderViewContext.m_pRenderContext->EndRendering();
    }

    // Lightning Pass
    if (const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex]; pOutput != nullptr)
    {
      // Setup render targets
      ezGALRenderingSetup renderingSetup;
      renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));

      renderViewContext.m_pRenderContext->BeginRendering(pPass, renderingSetup, renderViewContext.m_pViewData->m_ViewPortRect, "Lightning", renderViewContext.m_pCamera->IsStereoscopic());

      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      if (inputs[m_PinAlbedo.m_uiInputIndex])
      {
        renderViewContext.m_pRenderContext->BindTexture2D("AlbedoTexture", pDevice->GetDefaultResourceView(inputs[m_PinAlbedo.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinNormal.m_uiInputIndex])
      {
        renderViewContext.m_pRenderContext->BindTexture2D("NormalTexture", pDevice->GetDefaultResourceView(inputs[m_PinNormal.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinMaterial.m_uiInputIndex])
      {
        renderViewContext.m_pRenderContext->BindTexture2D("MaterialTexture", pDevice->GetDefaultResourceView(inputs[m_PinMaterial.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinVelocity.m_uiInputIndex])
      {
        renderViewContext.m_pRenderContext->BindTexture2D("VelocityTexture", pDevice->GetDefaultResourceView(inputs[m_PinVelocity.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinSSAO.m_uiInputIndex])
      {
        renderViewContext.m_pRenderContext->BindTexture2D("SSAOTexture", pDevice->GetDefaultResourceView(inputs[m_PinSSAO.m_uiInputIndex]->m_TextureHandle));
      }

      if (inputs[m_PinDepthStencil.m_uiInputIndex])
      {
        renderViewContext.m_pRenderContext->BindTexture2D("SceneDepth", pDevice->GetDefaultResourceView(inputs[m_PinDepthStencil.m_uiInputIndex]->m_TextureHandle));
      }

      const auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);
      pClusteredData->BindResources(renderViewContext.m_pRenderContext);

      renderViewContext.m_pRenderContext->BindMeshBuffer(ezGALBufferHandle(), ezGALBufferHandle(), nullptr, ezGALPrimitiveTopology::Triangles, 1);

      renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

      renderViewContext.m_pRenderContext->EndRendering();
    }
  }
  pDevice->EndPass(pPass);
}

void ezOpaqueDeferredRenderPass::SetupPermutationVars(const ezRenderViewContext& renderViewContext)
{
  ezTempHashedString sRenderPass("RENDER_PASS_DEFERRED");
  if (renderViewContext.m_pViewData->m_ViewRenderMode != ezViewRenderMode::None)
  {
    sRenderPass = ezViewRenderMode::GetPermutationValue(renderViewContext.m_pViewData->m_ViewRenderMode);
  }

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("RENDER_PASS", sRenderPass);

  ezStringBuilder sDebugText;
  ezViewRenderMode::GetDebugText(renderViewContext.m_pViewData->m_ViewRenderMode, sDebugText);
  if (!sDebugText.IsEmpty())
  {
    ezDebugRenderer::Draw2DText(*renderViewContext.m_pViewDebugContext, sDebugText, ezVec2I32(10, 10), ezColor::White);
  }

  if (m_bWriteDepth)
  {
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("FORWARD_PASS_WRITE_DEPTH", "TRUE");
  }
  else
  {
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("FORWARD_PASS_WRITE_DEPTH", "FALSE");
  }
}

void ezOpaqueDeferredRenderPass::RenderObjects(const ezRenderViewContext& renderViewContext)
{
  RenderDataWithCategory(renderViewContext, ezDefaultRenderDataCategories::LitOpaque);
  RenderDataWithCategory(renderViewContext, ezDefaultRenderDataCategories::LitMasked);
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_DeferredRenderPass);
