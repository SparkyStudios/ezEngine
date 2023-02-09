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

spResourceHandle spDeviceResourceFactoryD3D11::CreateShader(const spShaderDescription& description)
{
  spShaderD3D11* pShader = EZ_DEFAULT_NEW(spShaderD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pShader);
  return pShader->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateShaderProgram()
{
  spShaderProgramD3D11* pShaderProgram = EZ_DEFAULT_NEW(spShaderProgramD3D11, m_pDevice);
  m_pDevice->GetResourceManager()->RegisterResource(pShaderProgram);
  return pShaderProgram->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateTexture(const spTextureDescription& description)
{
  spTextureD3D11* pTexture = EZ_DEFAULT_NEW(spTextureD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pTexture);
  return pTexture->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateSampler(const spSamplerDescription& description)
{
  spSamplerD3D11* pSampler = EZ_DEFAULT_NEW(spSamplerD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pSampler);
  return pSampler->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateInputLayout(const spInputLayoutDescription& description, const spResourceHandle& hShader)
{
  spInputLayoutD3D11* pInputLayout = EZ_DEFAULT_NEW(spInputLayoutD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pInputLayout);
  return pInputLayout->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateBuffer(const spBufferDescription& description)
{
  spBufferD3D11* pBuffer = EZ_DEFAULT_NEW(spBufferD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pBuffer);
  return pBuffer->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateBufferRange(const spBufferRangeDescription& description)
{
  spBufferRangeD3D11* pBufferRange = EZ_DEFAULT_NEW(spBufferRangeD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pBufferRange);
  return pBufferRange->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateResourceLayout(const spResourceLayoutDescription& description)
{
  spResourceLayoutD3D11* pResourceLayout = EZ_DEFAULT_NEW(spResourceLayoutD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pResourceLayout);
  return pResourceLayout->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateTextureView(const spTextureViewDescription& description)
{
  spTextureViewD3D11* pTextureView = EZ_DEFAULT_NEW(spTextureViewD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pTextureView);
  return pTextureView->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateTextureView(const spResourceHandle& hTexture)
{
  auto* pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureD3D11>(hTexture);
  EZ_ASSERT_DEV(pTexture != nullptr, "Invalid texture handle.");

  spTextureViewD3D11* pTextureView = EZ_DEFAULT_NEW(spTextureViewD3D11, m_pDevice, spTextureViewDescription(pTexture));
  m_pDevice->GetResourceManager()->RegisterResource(pTextureView);
  return pTextureView->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateSwapchain(const spSwapchainDescription& description)
{
  spSwapchainD3D11* pSwapchain = EZ_DEFAULT_NEW(spSwapchainD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pSwapchain);
  return pSwapchain->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateFence(const spFenceDescription& description)
{
  spFenceD3D11* pFence = EZ_DEFAULT_NEW(spFenceD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pFence);
  return pFence->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateFramebuffer(const spFramebufferDescription& description)
{
  spFramebufferD3D11* pFramebuffer = EZ_DEFAULT_NEW(spFramebufferD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pFramebuffer);
  return pFramebuffer->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateCommandList(const spCommandListDescription& description)
{
  spCommandListD3D11* pCommandList = EZ_DEFAULT_NEW(spCommandListD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pCommandList);
  return pCommandList->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateComputePipeline(const spComputePipelineDescription& description)
{
  spComputePipelineD3D11* pComputePipeline = EZ_DEFAULT_NEW(spComputePipelineD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pComputePipeline);
  return pComputePipeline->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateGraphicPipeline(const spGraphicPipelineDescription& description)
{
  spGraphicPipelineD3D11* pGraphicPipeline = EZ_DEFAULT_NEW(spGraphicPipelineD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pGraphicPipeline);
  return pGraphicPipeline->GetHandle();
}

spResourceHandle spDeviceResourceFactoryD3D11::CreateResourceSet(const spResourceSetDescription& description)
{
  spResourceSetD3D11* pResourceSet = EZ_DEFAULT_NEW(spResourceSetD3D11, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pResourceSet);
  return pResourceSet->GetHandle();
}

spDeviceResourceFactoryD3D11::spDeviceResourceFactoryD3D11(spDeviceD3D11* pDevice)
  : spDeviceResourceFactory(pDevice)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();
}
