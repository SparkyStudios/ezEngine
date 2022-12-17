#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct ezAtomicCounterBuffer;

class EZ_RENDERERCORE_DLL ezBloomPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezBloomPass, ezRenderPipelinePass);

public:
  ezBloomPass();
  ~ezBloomPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;

  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateBloomConstantBuffer(ezVec2 pixelSize, ezUInt32 uiWorkGroupCount);
  void DownsamplePass(ezGALPass* pPass, const ezRenderViewContext& renderViewContext, const ezRenderPipelinePassConnection* pDownsampleTexture);

  ezRenderPipelineNodeInputPin m_PinInput;
  ezRenderPipelineNodeOutputPin m_PinBloomOutput;
  ezRenderPipelineNodeOutputPin m_PinBlendOutput;

  // Bloom shader data
  ezShaderResourceHandle m_hBloomShader;
  ezConstantBufferStorageHandle m_hBloomConstantBuffer;
  ezGALBufferHandle m_hDownsampleAtomicCounterBuffer;
  ezArrayPtr<ezAtomicCounterBuffer> m_DownsampleAtomicCounter;

  float m_fIntensity;
  ezUInt32 m_uiMipCount{};
};
