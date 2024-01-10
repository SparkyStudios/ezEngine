#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Rendering.h>
#include <RHI/Resource.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spDeviceResourceFactoryMTL final : public spDeviceResourceFactory
  {
    friend class spDeviceMTL;

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

    // spDeviceResourceFactoryMTL

  private:
    explicit spDeviceResourceFactoryMTL(spDeviceMTL* pDevice);

    spDeviceMTL* m_pDevice{nullptr};
    MTL::Device* m_pMTLDevice{nullptr};
  };
} // namespace RHI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHIMTL_DLL, RHI::spDeviceResourceFactoryMTL);
