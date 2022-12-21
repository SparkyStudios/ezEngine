#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct EZ_RENDERERCORE_DLL ezTonemapMode
{
  using StorageType = ezUInt8;

  enum Enum
  {
    AMD = 0,
    ACES,
    Reinhard,
    Uncharted2,
    None,

    Default = AMD
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezTonemapMode);

class EZ_RENDERERCORE_DLL ezTonemapPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTonemapPass, ezRenderPipelinePass);

public:
  ezTonemapPass();
  ~ezTonemapPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateConstantBuffer();

  ezRenderPipelineNodeInputPin m_PinInput;
  ezRenderPipelineNodeInputPin m_PinBloom;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  ezEnum<ezTonemapMode> m_eTonemapMode;

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hShader;
};