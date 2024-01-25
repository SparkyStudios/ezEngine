#pragma once

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

#include <RAI/Mesh.h>
#include <RAI/Resources/ImageResource.h>
#include <RAI/Resources/MeshResource.h>

#include <RHI/Buffer.h>
#include <RHI/CommandList.h>
#include <RHI/Device.h>
#include <RHI/Fence.h>
#include <RHI/Pipeline.h>
#include <RHI/Sampler.h>
#include <RHI/Shader.h>
#include <RHI/Swapchain.h>
#include <RHI/Texture.h>

#include <RPI/Core/RenderSystem.h>
#include <RPI/Core/Threading/RenderThread.h>
#include <RPI/Graph/RenderGraph.h>
#include <RPI/Pipeline/RenderPass.h>
#include <RPI/Pipeline/RenderPipeline.h>
#include <RPI/Scene/SceneContext.h>

#include <ShaderCompilerHLSL/ShaderCompilerHLSL.h>

using namespace RAI;
using namespace RHI;
using namespace RPI;

class ezRHISampleWindow;

class ezRHISampleApp : public ezApplication
{
public:
  typedef ezApplication SUPER;

  ezRHISampleApp();

  Execution Run() override;

  void AfterCoreSystemsStartup() override;

  void BeforeHighLevelSystemsShutdown() override;

  void AfterCoreSystemsShutdown() override;

  void OnResize(ezUInt32 width, ezUInt32 height);

private:
  ezUniquePtr<RPI::spRenderSystem> m_pRenderSystem{nullptr};

  ezRHISampleWindow* m_pWindow{nullptr};

  ezUniquePtr<spRenderGraphBuilder> graphBuilder;
  ezUniquePtr<spRenderPipeline> renderPipeline;

  const RAI::spMesh* m_pMesh{nullptr};
  RAI::spMeshResourceHandle m_hMesh;

  RAI::spTexture2DResourceHandle m_hTexture;
};

class spTriangleDemoRenderGraphNode final : public spRenderGraphNode
{
public:
  explicit spTriangleDemoRenderGraphNode(const RAI::spMesh* pMesh)
    : spRenderGraphNode("TrianglePass")
    , m_pMesh(pMesh)
  {
  }

  struct PassData
  {
    spResourceHandle m_hGridTexture;
    spResourceHandle m_hIndexBuffer;
    spResourceHandle m_hVertexBuffer;
    spResourceHandle m_hConstantBuffer;
    spResourceHandle m_hSampler;
    spResourceHandle m_hRenderTarget;
    spResourceHandle m_hIndirectBuffer;

    ezSharedPtr<spShader> m_pVertexShader{nullptr};
    ezSharedPtr<spShader> m_pPixelShader{nullptr};
    ezSharedPtr<spShaderProgram> m_pShaderProgram{nullptr};

    ezSharedPtr<spInputLayout> m_pInputLayout{nullptr};
    ezSharedPtr<spResourceLayout> m_pResourceLayout{nullptr};
    ezSharedPtr<spResourceSet> m_pResourceSet{nullptr};

    ezSharedPtr<spGraphicPipeline> m_pGraphicPipeline{nullptr};

    ezDynamicArray<spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper> m_DrawCommands;
  };

  EZ_NODISCARD EZ_ALWAYS_INLINE spResourceHandle GetTarget() const { return m_PassData.m_hRenderTarget; }

  ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources) override;
  ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) override;
  bool IsEnabled() const override;

private:
  const RAI::spMesh* m_pMesh{nullptr};
  PassData m_PassData;
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_NO_LINKAGE, spTriangleDemoRenderGraphNode::PassData);
EZ_DECLARE_CUSTOM_VARIANT_TYPE(spTriangleDemoRenderGraphNode::PassData);

void operator<<(ezStreamWriter& Stream, const spTriangleDemoRenderGraphNode::PassData& Value);
void operator>>(ezStreamReader& Stream, spTriangleDemoRenderGraphNode::PassData& Value);
bool operator==(const spTriangleDemoRenderGraphNode::PassData& lhs, const spTriangleDemoRenderGraphNode::PassData& rhs);

template <>
struct ezHashHelper<spTriangleDemoRenderGraphNode::PassData>
{
  EZ_ALWAYS_INLINE static ezUInt32 Hash(const spTriangleDemoRenderGraphNode::PassData& value)
  {
    ezUInt32 uiHash = 0;

    for (int i = 0; i < 6; ++i)
      uiHash += ezHashingUtils::xxHash32(reinterpret_cast<const ezUInt8*>(&value) + sizeof(spResourceHandle) * i, sizeof(spResourceHandle));

    return uiHash;
  }

  EZ_ALWAYS_INLINE static bool Equal(const spTriangleDemoRenderGraphNode::PassData& a, const spTriangleDemoRenderGraphNode::PassData& b) { return a == b; }
};
