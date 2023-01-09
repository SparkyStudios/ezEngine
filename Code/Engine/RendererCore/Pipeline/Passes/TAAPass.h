#pragma once

#include <Foundation/Reflection/Reflection.h>

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class EZ_RENDERERCORE_DLL ezTAAPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTAAPass, ezRenderPipelinePass);

public:
  ezTAAPass();
  ~ezTAAPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;

  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateTAAConstantBuffer() const;
  void UpdateCopyConstantBuffer(ezVec2I32 offset, ezVec2U32 size) const;

  ezRenderPipelineNodeInputPin m_PinInputColor;
  ezRenderPipelineNodeInputPin m_PinInputVelocity;
  ezRenderPipelineNodeInputPin m_PinInputHistory;
  ezRenderPipelineNodeInputPin m_PinInputDepth;
  ezRenderPipelineNodeOutputPin m_PinOutput;

  bool m_bUpsample;

  ezShaderResourceHandle m_hTAAShader;
  ezConstantBufferStorageHandle m_hTAAConstantBuffer;

  ezShaderResourceHandle m_hCopyShader;
  ezConstantBufferStorageHandle m_hCopyConstantBuffer;

  ezGALTextureHandle m_hPreviousVelocity;
  ezGALTextureHandle m_hHistory;
};
