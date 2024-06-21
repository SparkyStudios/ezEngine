#pragma once

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

#include <RAI/Mesh.h>
#include <RAI/Resources/Texture2DResource.h>
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
  ezUniquePtr<spSceneContext> m_pSceneContext{nullptr};

  ezRHISampleWindow* m_pWindow{nullptr};

  ezUniquePtr<spRenderGraphBuilder> graphBuilder;

  const RAI::spMesh* m_pMesh{nullptr};
  RAI::spMeshResourceHandle m_hMesh;

  RAI::spTexture2DResourceHandle m_hTexture;
};

class spDemoRenderGraphNode final : public spRenderGraphNode
{
public:
  explicit spDemoRenderGraphNode(const RAI::spMesh* pMesh)
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

    ezSharedPtr<RHI::spShader> m_pVertexShader{nullptr};
    ezSharedPtr<RHI::spShader> m_pPixelShader{nullptr};
    ezSharedPtr<spShaderProgram> m_pShaderProgram{nullptr};

    ezSharedPtr<spInputLayout> m_pInputLayout{nullptr};
    ezSharedPtr<spResourceLayout> m_pResourceLayout{nullptr};
    ezSharedPtr<spResourceSet> m_pResourceSet{nullptr};

    ezSharedPtr<spGraphicPipeline> m_pGraphicPipeline{nullptr};

    ezDynamicArray<spDrawIndexedIndirectCommand, ezAlignedAllocatorWrapper> m_DrawCommands;
  };

  [[nodiscard]] EZ_ALWAYS_INLINE spResourceHandle GetTarget() const { return m_PassData.m_hRenderTarget; }

  ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, spResourceHandle>& resources) override;
  ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) override;
  bool IsEnabled() const override;

private:
  const RAI::spMesh* m_pMesh{nullptr};
  PassData m_PassData;

  RAI::spShaderResourceHandle m_hShader;
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_NO_LINKAGE, spDemoRenderGraphNode::PassData);
EZ_DECLARE_CUSTOM_VARIANT_TYPE(spDemoRenderGraphNode::PassData);

void operator<<(ezStreamWriter& Stream, const spDemoRenderGraphNode::PassData& Value);
void operator>>(ezStreamReader& Stream, spDemoRenderGraphNode::PassData& Value);
bool operator==(const spDemoRenderGraphNode::PassData& lhs, const spDemoRenderGraphNode::PassData& rhs);

template <>
struct ezHashHelper<spDemoRenderGraphNode::PassData>
{
  EZ_ALWAYS_INLINE static ezUInt32 Hash(const spDemoRenderGraphNode::PassData& value)
  {
    ezUInt32 uiHash = 0;

    for (int i = 0; i < 6; ++i)
      uiHash += ezHashingUtils::xxHash32(reinterpret_cast<const ezUInt8*>(&value) + sizeof(spResourceHandle) * i, sizeof(spResourceHandle));

    return uiHash;
  }

  EZ_ALWAYS_INLINE static bool Equal(const spDemoRenderGraphNode::PassData& a, const spDemoRenderGraphNode::PassData& b) { return a == b; }
};
