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
  spResourceHandle CreateShader(const spShaderDescription& description) override;
  spResourceHandle CreateShaderProgram() override;
  spResourceHandle CreateTexture(const spTextureDescription& description) override;
  spResourceHandle CreateSampler(const spSamplerDescription& description) override;
  spResourceHandle CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader) override;
  spResourceHandle CreateBuffer(const spBufferDescription& description) override;
  spResourceHandle CreateBufferRange(const spBufferRangeDescription& description) override;
  spResourceHandle CreateResourceLayout(const spResourceLayoutDescription& description) override;
  spResourceHandle CreateTextureView(const spTextureViewDescription& description) override;
  spResourceHandle CreateTextureView(const spResourceHandle& hTexture) override;
  spResourceHandle CreateSwapchain(const spSwapchainDescription& description) override;
  spResourceHandle CreateFence(const spFenceDescription& description) override;
  spResourceHandle CreateFramebuffer(const spFramebufferDescription& description) override;
  spResourceHandle CreateCommandList(const spCommandListDescription& description) override;
  spResourceHandle CreateComputePipeline(const spComputePipelineDescription& description) override;
  spResourceHandle CreateGraphicPipeline(const spGraphicPipelineDescription& description) override;
  spResourceHandle CreateResourceSet(const spResourceSetDescription& description) override;

  // spDeviceResourceFactoryD3D11

private:
  explicit spDeviceResourceFactoryD3D11(spDeviceD3D11* pDevice);

  spDeviceD3D11* m_pDevice{nullptr};
  ID3D11Device* m_pD3D11Device{nullptr};
};
