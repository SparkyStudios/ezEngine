#include <RPI/RPIPCH.h>

#include <RPI/Pipeline/Passes/MainSwapchainRenderPass.h>

typedef spMainSwapchainRenderPass::Data spMainSwapchainRenderGraphNodePassData;

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spMainSwapchainRenderGraphNodePassData, ezNoBase, 1, ezRTTIDefaultAllocator<spMainSwapchainRenderGraphNodePassData>)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

spMainSwapchainRenderPass::spMainSwapchainRenderPass(Data&& passData)
  : spRenderPass({}, {})
  , m_PassData(passData)
{
}

void spMainSwapchainRenderPass::Execute(const spRenderGraphResourcesTable& resources, spRenderingContext* context)
{
  const auto cl = context->GetCommandList();

  spRenderGraphResource* input = nullptr;
  resources.TryGetValue(m_PassData.m_hInputTexture.GetInternalID(), input);

  const auto output = context->GetDevice()->GetResourceManager()->GetResource<spTexture>(context->GetDevice()->GetMainSwapchain()->GetFramebuffer()->GetColorTargets()[0]);

  cl->PushDebugGroup("Main Swapchain Pass");
  {
    cl->SetFramebuffer(context->GetDevice()->GetMainSwapchain()->GetFramebuffer());
    cl->CopyTexture(input->m_pResource.Downcast<spRenderTarget>()->GetTexture(), output);
  }
  cl->PopDebugGroup();
}

void spMainSwapchainRenderPass::CleanUp(const spRenderGraphResourcesTable& resources)
{
  // noop
}
