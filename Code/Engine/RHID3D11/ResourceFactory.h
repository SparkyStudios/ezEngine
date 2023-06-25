#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Rendering.h>
#include <RHI/Resource.h>

namespace RHI
{
  class spDeviceD3D11;

  class SP_RHID3D11_DLL spDeviceResourceFactoryD3D11 final : public spDeviceResourceFactory
  {
    friend class spDeviceD3D11;

    // spDeviceResourceFactory

  public:
    ezSharedPtr<spShader> CreateShader(const spShaderDescription& description) override;
    ezSharedPtr<spShaderProgram> CreateShaderProgram() override;
    ezSharedPtr<spTexture> CreateTexture(const spTextureDescription& description) override;
    ezSharedPtr<spSampler> CreateSampler(const spSamplerDescription& description) override;
    ezSharedPtr<spInputLayout> CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader) override;
    ezSharedPtr<spBuffer> CreateBuffer(const spBufferDescription& description) override;
    ezSharedPtr<spBufferRange> CreateBufferRange(const spBufferRangeDescription& description) override;
    ezSharedPtr<spResourceLayout> CreateResourceLayout(const spResourceLayoutDescription& description) override;
    ezSharedPtr<spTextureView> CreateTextureView(const spTextureViewDescription& description) override;
    ezSharedPtr<spTextureView> CreateTextureView(const spResourceHandle& hTexture) override;
    ezSharedPtr<spSwapchain> CreateSwapchain(const spSwapchainDescription& description) override;
    ezSharedPtr<spFence> CreateFence(const spFenceDescription& description) override;
    ezSharedPtr<spFramebuffer> CreateFramebuffer(const spFramebufferDescription& description) override;
    ezSharedPtr<spCommandList> CreateCommandList(const spCommandListDescription& description) override;
    ezSharedPtr<spComputePipeline> CreateComputePipeline(const spComputePipelineDescription& description) override;
    ezSharedPtr<spGraphicPipeline> CreateGraphicPipeline(const spGraphicPipelineDescription& description) override;
    ezSharedPtr<spResourceSet> CreateResourceSet(const spResourceSetDescription& description) override;

    // spDeviceResourceFactoryD3D11

  private:
    explicit spDeviceResourceFactoryD3D11(spDeviceD3D11* pDevice);

    spDeviceD3D11* m_pDevice{nullptr};
    ID3D11Device* m_pD3D11Device{nullptr};
  };
} // namespace RHI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, RHI::spDeviceResourceFactoryD3D11);
