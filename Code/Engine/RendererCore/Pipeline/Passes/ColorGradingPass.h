#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/Textures/Texture3DResource.h>

class EZ_RENDERERCORE_DLL ezColorGradingPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezColorGradingPass, ezRenderPipelinePass);

public:
  ezColorGradingPass();
  ~ezColorGradingPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodeInputPin m_PinInput;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  //  void SetVignettingTextureFile(const char* szFile);
  //  const char* GetVignettingTextureFile() const;

  void SetLUT1TextureFile(const char* szFile);
  const char* GetLUT1TextureFile() const;

  void SetLUT2TextureFile(const char* szFile);
  const char* GetLUT2TextureFile() const;

  ezTexture3DResourceHandle m_hLUT1;
  ezTexture3DResourceHandle m_hLUT2;

  ezColor m_MoodColor;
  float m_fMoodStrength;
  float m_fSaturation;
  float m_fContrast;
  float m_fLut1Strength;
  float m_fLut2Strength;

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hShader;
};
