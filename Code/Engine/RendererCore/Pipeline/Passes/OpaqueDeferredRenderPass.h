#pragma once

#include <RendererCore/Shader/ShaderResource.h>

/// \brief A deferred render pass that renders all opaque objects into multiple targets.
class EZ_RENDERERCORE_DLL ezOpaqueDeferredRenderPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezOpaqueDeferredRenderPass, ezRenderPipelinePass);

public:
  ezOpaqueDeferredRenderPass(const char* szName = "OpaqueDeferredRenderPass");
  ~ezOpaqueDeferredRenderPass() override;

  bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  void SetupPermutationVars(const ezRenderViewContext& renderViewContext);

  void RenderObjects(const ezRenderViewContext& renderViewContext);

  ezRenderPipelineNodeInputPin m_PinSSAO;

  ezRenderPipelineNodePassThrougPin m_PinAlbedo;
  ezRenderPipelineNodePassThrougPin m_PinNormal;
  ezRenderPipelineNodePassThrougPin m_PinMaterial;
  ezRenderPipelineNodePassThrougPin m_PinVelocity;
  ezRenderPipelineNodePassThrougPin m_PinDepthStencil;

  ezRenderPipelineNodeOutputPin m_PinOutput;

  ezShaderResourceHandle m_hShader;

  bool m_bWriteDepth;
};
