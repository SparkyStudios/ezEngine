#pragma once

#include <Foundation/Reflection/Reflection.h>

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct EZ_RENDERERCORE_DLL ezEdgeThresholdQuality
{
  using StorageType = ezUInt8;

  enum Enum : StorageType
  {
    Little,
    LowQuality,
    DefaultQuality,
    HighQuality,
    Overkill,

    Default = DefaultQuality,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezEdgeThresholdQuality);

struct EZ_RENDERERCORE_DLL ezEdgeThresholdMinQuality
{
  using StorageType = ezUInt8;

  enum Enum : StorageType
  {
    UpperLimit,
    HighQuality,
    VisibleLimit,

    Default = UpperLimit,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezEdgeThresholdMinQuality);

class EZ_RENDERERCORE_DLL ezFXAAPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezFXAAPass, ezRenderPipelinePass);

public:
  ezFXAAPass();
  ~ezFXAAPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;

  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateConstantBuffer() const;

  ezRenderPipelineNodeInputPin m_PinInput;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  float m_fSPARAmount;
  ezEnum<ezEdgeThresholdQuality> m_eEdgeThreshold;
  ezEnum<ezEdgeThresholdMinQuality> m_eEdgeThresholdMin;

  ezShaderResourceHandle m_hShader;
  ezConstantBufferStorageHandle m_hConstantBuffer;
};
