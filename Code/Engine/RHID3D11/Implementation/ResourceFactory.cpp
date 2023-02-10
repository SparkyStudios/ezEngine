#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/ResourceFactory.h>

#include <RHID3D11/Buffer.h>
#include <RHID3D11/CommandList.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Fence.h>
#include <RHID3D11/Framebuffer.h>
#include <RHID3D11/InputLayout.h>
#include <RHID3D11/Pipeline.h>
#include <RHID3D11/ResourceLayout.h>
#include <RHID3D11/ResourceSet.h>
#include <RHID3D11/Sampler.h>
#include <RHID3D11/Shader.h>
#include <RHID3D11/Swapchain.h>
#include <RHID3D11/Texture.h>

ezSharedPtr<spShader> spDeviceResourceFactoryD3D11::CreateShader(const spShaderDescription& description)
{
  ezSharedPtr<spShader> pShader = EZ_NEW(m_pDevice->GetAllocator(), spShaderD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pShader);
  return pShader;
}

ezSharedPtr<spShaderProgram> spDeviceResourceFactoryD3D11::CreateShaderProgram()
{
  ezSharedPtr<spShaderProgramD3D11> pShaderProgram = EZ_NEW(m_pDevice->GetAllocator(), spShaderProgramD3D11, m_pDevice);
  m_pDevice->GetResourceManager()->RegisterResource(pShaderProgram);
  return pShaderProgram;
}

ezSharedPtr<spTexture> spDeviceResourceFactoryD3D11::CreateTexture(const spTextureDescription& description)
{
  ezSharedPtr<spTextureD3D11> pTexture = EZ_NEW(m_pDevice->GetAllocator(), spTextureD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pTexture);
  return pTexture;
}

ezSharedPtr<spSampler> spDeviceResourceFactoryD3D11::CreateSampler(const spSamplerDescription& description)
{
  ezSharedPtr<spSamplerD3D11> pSampler = EZ_NEW(m_pDevice->GetAllocator(), spSamplerD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pSampler);
  return pSampler;
}

ezSharedPtr<spInputLayout> spDeviceResourceFactoryD3D11::CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader)
{
  ezSharedPtr<spInputLayoutD3D11> pInputLayout = EZ_NEW(m_pDevice->GetAllocator(), spInputLayoutD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pInputLayout);
  return pInputLayout;
}

ezSharedPtr<spBuffer> spDeviceResourceFactoryD3D11::CreateBuffer(const spBufferDescription& description)
{
  ezSharedPtr<spBufferD3D11> pBuffer = EZ_NEW(m_pDevice->GetAllocator(), spBufferD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pBuffer);
  return pBuffer;
}

ezSharedPtr<spBufferRange> spDeviceResourceFactoryD3D11::CreateBufferRange(const spBufferRangeDescription& description)
{
  ezSharedPtr<spBufferRangeD3D11> pBufferRange = EZ_NEW(m_pDevice->GetAllocator(), spBufferRangeD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pBufferRange);
  return pBufferRange;
}

ezSharedPtr<spResourceLayout> spDeviceResourceFactoryD3D11::CreateResourceLayout(const spResourceLayoutDescription& description)
{
  ezSharedPtr<spResourceLayoutD3D11> pResourceLayout = EZ_NEW(m_pDevice->GetAllocator(), spResourceLayoutD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pResourceLayout);
  return pResourceLayout;
}

ezSharedPtr<spTextureView> spDeviceResourceFactoryD3D11::CreateTextureView(const spTextureViewDescription& description)
{
  ezSharedPtr<spTextureViewD3D11> pTextureView = EZ_NEW(m_pDevice->GetAllocator(), spTextureViewD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pTextureView);
  return pTextureView;
}

ezSharedPtr<spTextureView> spDeviceResourceFactoryD3D11::CreateTextureView(const spResourceHandle& hTexture)
{
  auto pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(hTexture);
  EZ_ASSERT_DEV(pTexture != nullptr, "Invalid texture handle.");

  ezSharedPtr<spTextureViewD3D11> pTextureView = EZ_NEW(m_pDevice->GetAllocator(), spTextureViewD3D11, m_pDevice, spTextureViewDescription(pTexture));
  m_pDevice->GetResourceManager()->RegisterResource(pTextureView);
  return pTextureView;
}

ezSharedPtr<spSwapchain> spDeviceResourceFactoryD3D11::CreateSwapchain(const spSwapchainDescription& description)
{
  ezSharedPtr<spSwapchainD3D11> pSwapchain = EZ_NEW(m_pDevice->GetAllocator(), spSwapchainD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pSwapchain);
  return pSwapchain;
}

ezSharedPtr<spFence> spDeviceResourceFactoryD3D11::CreateFence(const spFenceDescription& description)
{
  ezSharedPtr<spFenceD3D11> pFence = EZ_NEW(m_pDevice->GetAllocator(), spFenceD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pFence);
  return pFence;
}

ezSharedPtr<spFramebuffer> spDeviceResourceFactoryD3D11::CreateFramebuffer(const spFramebufferDescription& description)
{
  ezSharedPtr<spFramebufferD3D11> pFramebuffer = EZ_NEW(m_pDevice->GetAllocator(), spFramebufferD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pFramebuffer);
  return pFramebuffer;
}

ezSharedPtr<spCommandList> spDeviceResourceFactoryD3D11::CreateCommandList(const spCommandListDescription& description)
{
  ezSharedPtr<spCommandListD3D11> pCommandList = EZ_NEW(m_pDevice->GetAllocator(), spCommandListD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pCommandList);
  return pCommandList;
}

ezSharedPtr<spComputePipeline> spDeviceResourceFactoryD3D11::CreateComputePipeline(const spComputePipelineDescription& description)
{
  ezSharedPtr<spComputePipelineD3D11> pComputePipeline = EZ_NEW(m_pDevice->GetAllocator(), spComputePipelineD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pComputePipeline);
  return pComputePipeline;
}

ezSharedPtr<spGraphicPipeline> spDeviceResourceFactoryD3D11::CreateGraphicPipeline(const spGraphicPipelineDescription& description)
{
  ezSharedPtr<spGraphicPipelineD3D11> pGraphicPipeline = EZ_NEW(m_pDevice->GetAllocator(), spGraphicPipelineD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pGraphicPipeline);
  return pGraphicPipeline;
}

ezSharedPtr<spResourceSet> spDeviceResourceFactoryD3D11::CreateResourceSet(const spResourceSetDescription& description)
{
  ezSharedPtr<spResourceSetD3D11> pResourceSet = EZ_NEW(m_pDevice->GetAllocator(), spResourceSetD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pResourceSet);
  return pResourceSet;
}

spDeviceResourceFactoryD3D11::spDeviceResourceFactoryD3D11(spDeviceD3D11* pDevice)
  : spDeviceResourceFactory(pDevice)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();
}
