#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Pipeline/RenderPass.h>

#include <RHI/Core.h>

class SP_RPI_DLL spMainSwapchainRenderPass : public spRenderPass
{
  // spRenderPass

public:
  void Execute(const spRenderGraphResourcesTable& resources, spRenderingContext* context) override;
  void CleanUp(const spRenderGraphResourcesTable& resources) override;

  // spMainSwapchainRenderPass

public:
  struct Data
  {
    EZ_DECLARE_POD_TYPE();

    RHI::spResourceHandle m_hInputTexture;
  };

  spMainSwapchainRenderPass(Data&& passData);

private:
  Data m_PassData;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, spMainSwapchainRenderPass::Data);
