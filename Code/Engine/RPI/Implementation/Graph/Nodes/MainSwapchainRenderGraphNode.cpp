#include <RPI/RPIPCH.h>

#include <RHI/RenderTarget.h>

#include <RPI/Core/RenderContext.h>
#include <RPI/Graph/Nodes/MainSwapchainRenderGraphNode.h>
#include <RPI/Pipeline/Passes/MainSwapchainRenderPass.h>

using namespace RHI;

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMainSwapchainRenderGraphNode, 1, ezRTTIDefaultAllocator<spMainSwapchainRenderGraphNode>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
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
    return EZ_NEW(pBuilder->GetAllocator(), spMainSwapchainRenderPass, std::move(m_PassData));
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
    SetRenderTargetSize(size.width, size.height);
  }

  void spMainSwapchainRenderGraphNode::SetRenderTargetSize(ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    m_Size.height = uiHeight;
    m_Size.width = uiWidth;
  }
}