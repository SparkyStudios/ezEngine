#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/ResourceFactory.h>

#include <RHIMTL/Buffer.h>
#include <RHIMTL/CommandList.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Fence.h>
#include <RHIMTL/Framebuffer.h>
#include <RHIMTL/InputLayout.h>
#include <RHIMTL/Pipeline.h>
#include <RHIMTL/ResourceLayout.h>
#include <RHIMTL/ResourceSet.h>
#include <RHIMTL/Sampler.h>
#include <RHIMTL/Shader.h>
#include <RHIMTL/Swapchain.h>
#include <RHIMTL/Texture.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(RHI::spDeviceResourceFactoryMTL, RHI::spDeviceResourceFactory, 1, ezRTTINoAllocator)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace RHI
{
  ezSharedPtr<spShader> spDeviceResourceFactoryMTL::CreateShader(const spShaderDescription& description)
  {
    ezSharedPtr<spShader> pShader = EZ_NEW(m_pDevice->GetAllocator(), spShaderMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pShader);
    return pShader;
  }

  ezSharedPtr<spShaderProgram> spDeviceResourceFactoryMTL::CreateShaderProgram()
  {
    ezSharedPtr<spShaderProgramMTL> pShaderProgram = EZ_NEW(m_pDevice->GetAllocator(), spShaderProgramMTL, m_pDevice);
    m_pDevice->GetResourceManager()->RegisterResource(pShaderProgram);
    return pShaderProgram;
  }

  ezSharedPtr<spTexture> spDeviceResourceFactoryMTL::CreateTexture(const spTextureDescription& description)
  {
    ezSharedPtr<spTextureMTL> pTexture = EZ_NEW(m_pDevice->GetAllocator(), spTextureMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pTexture);
    return pTexture;
  }

  ezSharedPtr<spSampler> spDeviceResourceFactoryMTL::CreateSampler(const spSamplerDescription& description)
  {
    ezSharedPtr<spSamplerMTL> pSampler = EZ_NEW(m_pDevice->GetAllocator(), spSamplerMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pSampler);
    return pSampler;
  }

  ezSharedPtr<spInputLayout> spDeviceResourceFactoryMTL::CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader)
  {
    ezSharedPtr<spInputLayoutMTL> pInputLayout = EZ_NEW(m_pDevice->GetAllocator(), spInputLayoutMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pInputLayout);
    return pInputLayout;
  }

  ezSharedPtr<spBuffer> spDeviceResourceFactoryMTL::CreateBuffer(const spBufferDescription& description)
  {
    ezSharedPtr<spBufferMTL> pBuffer = EZ_NEW(m_pDevice->GetAllocator(), spBufferMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pBuffer);
    return pBuffer;
  }

  ezSharedPtr<spBufferRange> spDeviceResourceFactoryMTL::CreateBufferRange(const spBufferRangeDescription& description)
  {
    ezSharedPtr<spBufferRangeMTL> pBufferRange = EZ_NEW(m_pDevice->GetAllocator(), spBufferRangeMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pBufferRange);
    return pBufferRange;
  }

  ezSharedPtr<spResourceLayout> spDeviceResourceFactoryMTL::CreateResourceLayout(const spResourceLayoutDescription& description)
  {
    ezSharedPtr<spResourceLayoutMTL> pResourceLayout = EZ_NEW(m_pDevice->GetAllocator(), spResourceLayoutMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pResourceLayout);
    return pResourceLayout;
  }

  ezSharedPtr<spTextureView> spDeviceResourceFactoryMTL::CreateTextureView(const spTextureViewDescription& description)
  {
    ezSharedPtr<spTextureViewMTL> pTextureView = EZ_NEW(m_pDevice->GetAllocator(), spTextureViewMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pTextureView);
    return pTextureView;
  }

  ezSharedPtr<spTextureView> spDeviceResourceFactoryMTL::CreateTextureView(const spResourceHandle& hTexture)
  {
    ezSharedPtr<spTextureViewMTL> pTextureView = EZ_NEW(m_pDevice->GetAllocator(), spTextureViewMTL, m_pDevice, spTextureViewDescription(hTexture));
    m_pDevice->GetResourceManager()->RegisterResource(pTextureView);
    return pTextureView;
  }

  ezSharedPtr<spSwapchain> spDeviceResourceFactoryMTL::CreateSwapchain(const spSwapchainDescription& description)
  {
    ezSharedPtr<spSwapchainMTL> pSwapchain = EZ_NEW(m_pDevice->GetAllocator(), spSwapchainMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pSwapchain);
    return pSwapchain;
  }

  ezSharedPtr<spFence> spDeviceResourceFactoryMTL::CreateFence(const spFenceDescription& description)
  {
    ezSharedPtr<spFenceMTL> pFence = EZ_NEW(m_pDevice->GetAllocator(), spFenceMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pFence);
    return pFence;
  }

  ezSharedPtr<spFramebuffer> spDeviceResourceFactoryMTL::CreateFramebuffer(const spFramebufferDescription& description)
  {
    ezSharedPtr<spFramebufferMTL> pFramebuffer = EZ_NEW(m_pDevice->GetAllocator(), spFramebufferMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pFramebuffer);
    return pFramebuffer;
  }

  ezSharedPtr<spCommandList> spDeviceResourceFactoryMTL::CreateCommandList(const spCommandListDescription& description)
  {
    ezSharedPtr<spCommandListMTL> pCommandList = EZ_NEW(m_pDevice->GetAllocator(), spCommandListMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pCommandList);
    return pCommandList;
  }

  ezSharedPtr<spComputePipeline> spDeviceResourceFactoryMTL::CreateComputePipeline(const spComputePipelineDescription& description)
  {
    ezSharedPtr<spComputePipelineMTL> pComputePipeline = EZ_NEW(m_pDevice->GetAllocator(), spComputePipelineMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pComputePipeline);
    return pComputePipeline;
  }

  ezSharedPtr<spGraphicPipeline> spDeviceResourceFactoryMTL::CreateGraphicPipeline(const spGraphicPipelineDescription& description)
  {
    ezSharedPtr<spGraphicPipelineMTL> pGraphicPipeline = EZ_NEW(m_pDevice->GetAllocator(), spGraphicPipelineMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pGraphicPipeline);
    return pGraphicPipeline;
  }

  ezSharedPtr<spResourceSet> spDeviceResourceFactoryMTL::CreateResourceSet(const spResourceSetDescription& description)
  {
    ezSharedPtr<spResourceSetMTL> pResourceSet = EZ_NEW(m_pDevice->GetAllocator(), spResourceSetMTL, m_pDevice, description);
    m_pDevice->GetResourceManager()->RegisterResource(pResourceSet);
    return pResourceSet;
  }

  spDeviceResourceFactoryMTL::spDeviceResourceFactoryMTL(spDeviceMTL* pDevice)
    : spDeviceResourceFactory(pDevice)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_ResourceFactory);
