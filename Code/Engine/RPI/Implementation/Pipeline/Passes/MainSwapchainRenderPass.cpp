#include <RPI/RPIPCH.h>

#include <RPI/Pipeline/Passes/MainSwapchainRenderPass.h>

using namespace RHI;

typedef RPI::spMainSwapchainRenderPass::Data spMainSwapchainRenderGraphNodePassData;

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spMainSwapchainRenderGraphNodePassData, ezNoBase, 1, ezRTTIDefaultAllocator<spMainSwapchainRenderGraphNodePassData>)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace RPI
{
  spMainSwapchainRenderPass::spMainSwapchainRenderPass(Data&& passData)
    : spRenderPass({}, {})
  {
    SetData(passData);
  }

  void spMainSwapchainRenderPass::Execute(const spRenderGraphResourcesTable& resources, spRenderContext* context)
  {
    const auto cl = context->GetCommandList();

    spRenderGraphResource* input = nullptr;
    resources.TryGetValue(m_PassData.Get<Data>().m_hInputTexture.GetInternalID(), input);

    auto const output = context->GetDevice()->GetResourceManager()->GetResource<spTexture>(context->GetDevice()->GetMainSwapchain()->GetFramebuffer()->GetColorTargets()[0]);

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
} // namespace RPI
