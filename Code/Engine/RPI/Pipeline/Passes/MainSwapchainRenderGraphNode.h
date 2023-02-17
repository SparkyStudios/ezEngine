#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Pipeline/Graph/RenderGraph.h>

#include <RHI/Core.h>

class SP_RPI_DLL spMainSwapchainRenderGraphNode final : public spRenderGraphNode
{
  EZ_ADD_DYNAMIC_REFLECTION(spMainSwapchainRenderGraphNode, spRenderGraphNode);

public:
  ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources) override;
  ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) override;
  bool IsEnabled() const override;

  // --- spMainSwapchainRenderGraphNode

public:
  struct PassData
  {
    EZ_DECLARE_POD_TYPE();

    spResourceHandle m_hInputTexture;
  };

  spMainSwapchainRenderGraphNode();

  void SetRenderTargetSize(ezSizeU32 size);

  void SetRenderTargetSize(ezUInt32 uiWidth, ezUInt32 uiHeight);

private:
  PassData m_PassData;

  ezSizeU32 m_Size;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, spMainSwapchainRenderGraphNode::PassData);
EZ_DECLARE_CUSTOM_VARIANT_TYPE(spMainSwapchainRenderGraphNode::PassData);

SP_RPI_DLL void operator<<(ezStreamWriter& Stream, const spMainSwapchainRenderGraphNode::PassData& Value);
SP_RPI_DLL void operator>>(ezStreamReader& Stream, spMainSwapchainRenderGraphNode::PassData& Value);
SP_RPI_DLL bool operator==(const spMainSwapchainRenderGraphNode::PassData& lhs, const spMainSwapchainRenderGraphNode::PassData& rhs);

template <>
struct ezHashHelper<spMainSwapchainRenderGraphNode::PassData>
{
  EZ_ALWAYS_INLINE static ezUInt32 Hash(const spMainSwapchainRenderGraphNode::PassData& value)
  {
    return ezHashingUtils::xxHash32(&value, sizeof(spResourceHandle));
  }

  EZ_ALWAYS_INLINE static bool Equal(const spMainSwapchainRenderGraphNode::PassData& a, const spMainSwapchainRenderGraphNode::PassData& b) { return a == b; }
};
