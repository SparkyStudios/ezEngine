#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Rendering.h>
#include <RHI/Resource.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spDeviceResourceFactoryD3D11 final : public spDeviceResourceFactory
{
  friend class spDeviceD3D11;

  // spDeviceResourceFactory

public:
  spShader* CreateShader(const spShaderDescription& description) override;
  spShaderProgram* CreateShaderProgram() override;
  spTexture* CreateTexture(const spTextureDescription& description) override;
  spSampler* CreateSampler(const spSamplerDescription& description) override;
  spInputLayout* CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader) override;
  spBuffer* CreateBuffer(const spBufferDescription& description) override;
  spBufferRange* CreateBufferRange(const spBufferRangeDescription& description) override;
  spResourceLayout* CreateResourceLayout(const spResourceLayoutDescription& description) override;
  spTextureView* CreateTextureView(const spTextureViewDescription& description) override;
  spTextureView* CreateTextureView(const spResourceHandle& hTexture) override;
  spSwapchain* CreateSwapchain(const spSwapchainDescription& description) override;
  spFence* CreateFence(const spFenceDescription& description) override;
  spFramebuffer* CreateFramebuffer(const spFramebufferDescription& description) override;
  spCommandList* CreateCommandList(const spCommandListDescription& description) override;
  spComputePipeline* CreateComputePipeline(const spComputePipelineDescription& description) override;
  spGraphicPipeline* CreateGraphicPipeline(const spGraphicPipelineDescription& description) override;
  spResourceSet* CreateResourceSet(const spResourceSetDescription& description) override;

  // spDeviceResourceFactoryD3D11

private:
  explicit spDeviceResourceFactoryD3D11(spDeviceD3D11* pDevice);

  spDeviceD3D11* m_pDevice{nullptr};
  ID3D11Device* m_pD3D11Device{nullptr};
};
