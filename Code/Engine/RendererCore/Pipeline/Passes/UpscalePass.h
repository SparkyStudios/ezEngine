#pragma once
#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct ezUpscaleMode
{
  using StorageType = ezUInt8;

  enum Enum
  {
    /**
     * Uses AMD Fidelity FX Super Resolution as the upscale method.
     * Over better results than linear for almost no costs on performances.
     */
    FSR,

    /**
     * Uses simple bi-linear filtering as the upscale method.
     * The fastest way, but with quality drops.
     */
    BiLinear,

    Default = FSR
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezUpscaleMode);

struct ezFSRUpscalePreset
{
  using StorageType = ezUInt8;

  enum Enum
  {
    /**
     * Ultra Quality preset produces an image with quality virtually indistinguishable from native
     * rendering. It should be selected when the highest quality is desired.
     */
    UltraQuality,

    /**
     * Quality preset produces a super resolution image with quality representative of native rendering,
     * with a sizeable performance gain.
     */
    Quality,

    /**
     * Balanced preset produces a super resolution image approximating native rendering quality, with a major
     * performance gain compared to native.
     */
    Balanced,

    /**
     * Performance preset visibly impacts image quality and should only be selected in situations where needing
     * additional performance is critical.
     */
    Performance,

    Default = UltraQuality
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezFSRUpscalePreset);

class EZ_RENDERERCORE_DLL ezUpscalePass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezUpscalePass, ezRenderPipelinePass);

public:
  ezUpscalePass();
  ~ezUpscalePass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;

  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodeInputPin m_PinInput;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  ezEnum<ezUpscaleMode> m_eUpscaleMode;

  ezEnum<ezFSRUpscalePreset> m_eFSRPreset;
  bool m_bFSRSharpen;
  float m_fFSRSharpness;

  ezShaderResourceHandle m_hShader;
  ezConstantBufferStorageHandle m_hConstantBuffer;
};
