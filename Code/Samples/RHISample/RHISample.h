#pragma once

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

#include <RAI/Mesh.h>
#include <RAI/Resources/MeshResource.h>
#include <RAI/Resources/ImageResource.h>

#include <RHI/Buffer.h>
#include <RHI/CommandList.h>
#include <RHI/Device.h>
#include <RHI/Fence.h>
#include <RHI/Pipeline.h>
#include <RHI/Sampler.h>
#include <RHI/Shader.h>
#include <RHI/Swapchain.h>
#include <RHI/Texture.h>

#include <RPI/Core/RenderingThread.h>
#include <RPI/Graph/RenderGraph.h>
#include <RPI/Pipeline/RenderPass.h>
#include <RPI/Pipeline/RenderPipeline.h>
#include <RPI/Scene/SceneContext.h>

#include <ShaderCompilerHLSL/ShaderCompilerHLSL.h>

using namespace RAI;
using namespace RHI;

class ezRHISampleWindow;

// ezString GetShaderPath(const ezString& shaderFile)
// {
//   ezStringBuilder projectDirAbsolutePath;
//   if (!ezFileSystem::ResolveSpecialDirectory(">project", projectDirAbsolutePath).Succeeded())
//   {
//     EZ_REPORT_FAILURE("Project directory could not be resolved.");
//     return {};
//   }

//   ezStringBuilder shaderPath(projectDirAbsolutePath, shaderFile);
//   shaderPath.MakeCleanPath();

//   return shaderPath.GetData();
// }

// struct VertexShaderDesc
// {
//   static constexpr ShaderType type = ShaderType::kVertex;
//   ShaderDesc desc = {GetShaderPath("/shaders/Triangle/VertexShader.hlsl"), "main", type, "6_0"};

//   struct IA
//   {
//     static constexpr const uint32_t POSITION = 0;
//   } ia;
// };

// class VertexShader : public VertexShaderDesc
// {
// public:
//   VertexShader(RenderDevice& device, ShaderBlobType shaderBlobType)
//     : m_ShaderBlobType{shaderBlobType}
//   {
//   }

//   void CompileShader(RenderDevice& device)
//   {
//     auto full_desc = desc;
//     ezDynamicArray<ezUInt8> byteCode = Compile(full_desc, m_ShaderBlobType);
//     ezSharedPtr<ShaderReflection> reflection = CreateShaderReflection(m_ShaderBlobType, byteCode.GetData(), byteCode.GetCount());
//     shader = device.CreateShader(full_desc, byteCode, reflection);
//   }

//   ezSharedPtr<Shader> shader;

// private:
//   ShaderBlobType m_ShaderBlobType;
// };

// struct PixelShaderDesc
// {
//   static constexpr ShaderType type = ShaderType::kPixel;
//   ShaderDesc desc = {GetShaderPath("/shaders/Triangle/PixelShader.hlsl"), "main", type, "6_0"};

//   struct CBV
//   {
//     BindKey Settings;
//   } cbv;

//   struct OM
//   {
//     static constexpr const uint32_t rtv0 = 0;
//   } om;
// };

// class PixelShader : public PixelShaderDesc
// {
// public:
//   struct Settings
//   {
//     ezColor color;
//   };
//   BufferLayout Settings_layout = {16, {
//                                         16,
//                                       },
//     {
//       offsetof(Settings, color),
//     },
//     {
//       0,
//     }};
//   struct Cbuffer
//   {
//     Cbuffer(PixelShader& shader, RenderDevice& device)
//       : Settings(device, shader.Settings_layout)
//     {
//     }
//     ConstantBuffer<Settings> Settings;
//   } cbuffer;

//   PixelShader(RenderDevice& device, ShaderBlobType shaderBlobType)
//     : cbuffer(*this, device)
//     , m_ShaderBlobType{shaderBlobType}
//   {
//   }

//   void CompileShader(RenderDevice& device)
//   {
//     auto full_desc = desc;
//     ezDynamicArray<ezUInt8> byteCode = Compile(full_desc, m_ShaderBlobType);
//     ezSharedPtr<ShaderReflection> reflection = CreateShaderReflection(m_ShaderBlobType, byteCode.GetData(), byteCode.GetCount());
//     shader = device.CreateShader(full_desc, byteCode, reflection);
//     cbv.Settings = shader->GetBindKey("Settings");
//   }

//   ezSharedPtr<Shader> shader;

// private:
//   ShaderBlobType m_ShaderBlobType;
// };

// using ProgramHolderType = ProgramHolder<PixelShader, VertexShader>;

class ezRHISampleApp : public ezApplication
{
public:
  typedef ezApplication SUPER;

  ezRHISampleApp();

  virtual Execution Run() override;

  virtual void AfterCoreSystemsStartup() override;

  virtual void BeforeHighLevelSystemsShutdown() override;

  virtual void AfterCoreSystemsShutdown() override;


  void OnResize(ezUInt32 width, ezUInt32 height);

private:
  ezRHISampleWindow* m_pWindow{nullptr};
  spRenderingThread* m_pRenderingThread{nullptr};

  ezSharedPtr<spDevice> m_pDevice{nullptr};
  ezUniquePtr<spSceneContext> m_pSceneContext{nullptr};

  ezUniquePtr<spRenderGraphBuilder> graphBuilder;
  ezUniquePtr<spRenderPipeline> renderPipeline;

  RAI::spMesh m_Mesh;
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
