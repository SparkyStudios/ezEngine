#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Pipeline/RenderPass.h>

#include <RHI/Core.h>

namespace RPI
{
  class SP_RPI_DLL spMainSwapchainRenderPass : public spRenderPass
  {
    // spRenderPass

  public:
    void Execute(const spRenderGraphResourcesTable& resources, spRenderContext* context) override;
    void CleanUp(const spRenderGraphResourcesTable& resources) override;

    // spMainSwapchainRenderPass

  public:
    struct Data
    {
      EZ_DECLARE_POD_TYPE();

      RHI::spResourceHandle m_hInputTexture;
    };

    explicit spMainSwapchainRenderPass(Data&& passData);
  };
} // namespace RPI

EZ_DECLARE_CUSTOM_VARIANT_TYPE(RPI::spMainSwapchainRenderPass::Data);

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spMainSwapchainRenderPass::Data);
