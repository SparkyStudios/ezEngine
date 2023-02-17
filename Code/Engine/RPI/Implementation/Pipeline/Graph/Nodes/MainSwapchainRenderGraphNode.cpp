#include <RPI/RPIPCH.h>

#include <RHI/RenderTarget.h>

#include <RPI/Core/RenderingContext.h>
#include <RPI/Pipeline/Passes/MainSwapchainRenderGraphNode.h>
#include <RPI/Pipeline/RenderPass.h>

#include <Foundation/Types/VariantTypeRegistry.h>

typedef spMainSwapchainRenderGraphNode::PassData spMainSwapchainRenderGraphNodePassData;

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMainSwapchainRenderGraphNode, 1, ezRTTIDefaultAllocator<spMainSwapchainRenderGraphNode>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_STATIC_REFLECTED_TYPE(spMainSwapchainRenderGraphNodePassData, ezNoBase, 1, ezRTTIDefaultAllocator<spMainSwapchainRenderGraphNodePassData>)
EZ_END_STATIC_REFLECTED_TYPE;

EZ_DEFINE_CUSTOM_VARIANT_TYPE(spMainSwapchainRenderGraphNodePassData);
// clang-format on

ezResult spMainSwapchainRenderGraphNode::Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources)
{
  if (!resources.TryGetValue("Input", m_PassData.m_hInputTexture))
    return EZ_FAILURE;

  m_PassData.m_hInputTexture = pBuilder->Read(this, m_PassData.m_hInputTexture);

  return EZ_SUCCESS;
}

ezUniquePtr<spRenderPass> spMainSwapchainRenderGraphNode::Compile(spRenderGraphBuilder* pBuilder)
{
  const spRenderPass::ExecuteCallback callback = [name = GetName()](const spRenderGraphResourcesTable& resources, spRenderingContext* context, ezVariant& passData) -> void
  {
    const auto cl = context->GetCommandList();
    const auto& data = passData.Get<PassData>();

    spRenderGraphResource* input = nullptr;
    resources.TryGetValue(data.m_hInputTexture.GetInternalID(), input);

    const auto output = context->GetDevice()->GetResourceManager()->GetResource<spTexture>(context->GetDevice()->GetMainSwapchain()->GetFramebuffer()->GetColorTargets()[0]);

    cl->PushDebugGroup(name);
    {
      cl->SetFramebuffer(context->GetDevice()->GetMainSwapchain()->GetFramebuffer());
      cl->CopyTexture(input->m_pResource.Downcast<spRenderTarget>()->GetTexture(), output);
    }
    cl->PopDebugGroup();
  };

  ezUniquePtr<spRenderPass> pPass = EZ_NEW(pBuilder->GetAllocator(), spRenderPass, callback);
  pPass->SetData(m_PassData);

  return pPass;
}

bool spMainSwapchainRenderGraphNode::IsEnabled() const
{
  return true;
}

spMainSwapchainRenderGraphNode::spMainSwapchainRenderGraphNode()
  : spRenderGraphNode("MainSwapchainPass")
{
}

void spMainSwapchainRenderGraphNode::SetRenderTargetSize(ezSizeU32 size)
{
  m_Size = size;
}

void spMainSwapchainRenderGraphNode::SetRenderTargetSize(ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  m_Size.height = uiHeight;
  m_Size.width = uiWidth;
}

void operator<<(ezStreamWriter& Stream, const spMainSwapchainRenderGraphNode::PassData& Value)
{
}

void operator>>(ezStreamReader& Stream, spMainSwapchainRenderGraphNode::PassData& Value)
{
}

bool operator==(const spMainSwapchainRenderGraphNode::PassData& lhs, const spMainSwapchainRenderGraphNode::PassData& rhs)
{
  return lhs.m_hInputTexture == rhs.m_hInputTexture;
}
