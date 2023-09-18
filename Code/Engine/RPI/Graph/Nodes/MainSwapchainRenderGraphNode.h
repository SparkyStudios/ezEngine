#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Graph/RenderGraph.h>
#include <RPI/Pipeline/Passes/MainSwapchainRenderPass.h>

#include <RHI/Core.h>

namespace RPI
{
  class SP_RPI_DLL spMainSwapchainRenderGraphNode final : public spRenderGraphNode
  {
    EZ_ADD_DYNAMIC_REFLECTION(spMainSwapchainRenderGraphNode, spRenderGraphNode);

  public:
    ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, RHI::spResourceHandle>& resources) override;
    ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) override;
    bool IsEnabled() const override;

    // --- spMainSwapchainRenderGraphNode

  public:
    spMainSwapchainRenderGraphNode();

    void SetRenderTargetSize(ezSizeU32 size);

    void SetRenderTargetSize(ezUInt32 uiWidth, ezUInt32 uiHeight);

  private:
    spMainSwapchainRenderPass::Data m_PassData;

    ezSizeU32 m_Size;
  };
} // namespace RPI
