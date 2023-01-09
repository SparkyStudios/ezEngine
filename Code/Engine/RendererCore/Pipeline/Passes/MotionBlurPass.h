#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct ezMotionBlurMode
{
  using StorageType = ezUInt8;

  enum Enum
  {
    /// @brief Applies motion blur on objects when they are moving and on camera movements.
    /// This won't affect the skybox since it's always static in the scene.
    /// @note This mode requires the Velocity texture as input.
    ObjectBased,

    /// @brief Applies motion blur on the whole screen only on camera movements.
    /// This will affect the skybox too.
    /// @note This mode requires the Depth texture as input.
    ScreenBased,

    Default = ObjectBased,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezMotionBlurMode);

class EZ_RENDERERCORE_DLL ezMotionBlurPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezMotionBlurPass, ezRenderPipelinePass);

public:
  ezMotionBlurPass();
  ~ezMotionBlurPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodeInputPin m_PinInputColor;
  ezRenderPipelineNodeInputPin m_PinInputVelocity;
  ezRenderPipelineNodeInputPin m_PinInputDepth;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  ezShaderResourceHandle m_hShader;
  ezConstantBufferStorageHandle m_hConstantBuffer;

  float m_fStrength;
  ezEnum<ezMotionBlurMode> m_eMode;
};
