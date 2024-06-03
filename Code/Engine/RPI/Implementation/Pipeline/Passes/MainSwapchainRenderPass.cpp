#include <RPI/RPIPCH.h>

#include <RPI/Pipeline/Passes/MainSwapchainRenderPass.h>
#include <RPI/Scene/SceneContext.h>

using namespace RHI;

typedef RPI::spMainSwapchainRenderPass::Data spMainSwapchainRenderGraphNodePassData;

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spMainSwapchainRenderGraphNodePassData, ezNoBase, 1, ezRTTIDefaultAllocator<spMainSwapchainRenderGraphNodePassData>)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace RPI
{
  spMainSwapchainRenderPass::spMainSwapchainRenderPass(Data&& passData)
  {
    SetData(passData);
  }

  void spMainSwapchainRenderPass::Execute(const spRenderGraphResourcesTable& resources, const spRenderContext* context)
  {
    const auto cl = context->GetCommandList();
    const auto& data = m_PassData.Get<Data>();

    spRenderGraphResource* input = nullptr;
    resources.TryGetValue(data.m_hInputTexture.GetInternalID(), input);

    auto pDevice = context->GetSceneContext()->GetDevice();
    auto pFramebuffer = pDevice->GetMainSwapchain()->GetFramebuffer();

    auto const output = pDevice->GetResourceManager()->GetResource<spTexture>(pFramebuffer->GetColorTargets()[data.m_uiColorAttachmentIndex]);

    cl->PushDebugGroup("Main Swapchain Pass");
    {
      cl->SetFramebuffer(pFramebuffer);
      cl->CopyTexture(input->m_pRHIResource.Downcast<spRenderTarget>()->GetTexture(), output);
    }
    cl->PopDebugGroup();
  }

  void spMainSwapchainRenderPass::  CleanUp(const spRenderGraphResourcesTable& resources)
  {
    // noop
  }
} // namespace RPI
