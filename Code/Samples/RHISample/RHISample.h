#pragma once

#include <Foundation/Application/Application.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

#include <RHI/Buffer.h>
#include <RHI/Device.h>
#include <RHI/Fence.h>
#include <RHI/Pipeline.h>
#include <RHI/Sampler.h>
#include <RHI/Swapchain.h>
#include <RHI/Texture.h>

#include <RPI/Core/RenderingThread.h>

#include <ShaderCompilerHLSL/ShaderCompilerHLSL.h>

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

  void OnResize(ezUInt32 width, ezUInt32 height);

private:
  ezRHISampleWindow* m_pWindow{nullptr};
  spRenderingThread* m_pRenderingThread{nullptr};

  ezSharedPtr<spDevice> device;

  ezSharedPtr<spGraphicPipeline> gpo;
  ezSharedPtr<spBuffer> ibo;
  ezSharedPtr<spBuffer> vbo;
  ezSharedPtr<spBuffer> cbo;
  ezSharedPtr<spTexture> tex;
  ezSharedPtr<spSampler> smp;

  ezSharedPtr<spInputLayout> input;
  ezSharedPtr<spResourceLayout> layout;
  ezSharedPtr<spResourceSet> set;

  ezSharedPtr<spShader> vs;
  ezSharedPtr<spShader> ps;
  ezSharedPtr<spShaderProgram> spo;

  ezSharedPtr<spCommandList> cl;
};
