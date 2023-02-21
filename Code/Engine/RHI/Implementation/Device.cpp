#include <RHI/RHIPCH.h>

#include <RHI/Device.h>

#pragma region spDevice

const spMappedResource g_InvalidMappedResource = {};

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spDevice, ezReflectedClass, 1, ezRTTINoAllocator)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

spDevice::spDevice(ezAllocatorBase* pAllocator, spDeviceDescription description)
  : m_Description(std::move(description))
  , m_pAllocator(pAllocator)
{
}

void spDevice::SubmitCommandList(ezSharedPtr<spCommandList> pCommandList)
{
  SubmitCommandList(pCommandList, nullptr);
}

bool spDevice::WaitForFence(ezSharedPtr<spFence> pFence, const ezTime& timeout)
{
  return WaitForFence(pFence, timeout.GetNanoseconds());
}

bool spDevice::WaitForFences(const ezList<ezSharedPtr<spFence>>& fences, bool bWaitAll, const ezTime& timeout)
{
  return WaitForFences(fences, bWaitAll, timeout.GetNanoseconds());
}

void spDevice::ResizeSwapchain(ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  spSwapchain* pSwapchain = GetMainSwapchain();

  if (pSwapchain == nullptr || pSwapchain->GetHandle().IsInvalidated())
    return;

  pSwapchain->Resize(uiWidth, uiHeight);
}

void spDevice::WaitForIdle()
{
  WaitForIdleInternal();
  m_pResourceManager->ReleaseResources();
}

const spMappedResource& spDevice::Map(ezSharedPtr<spMappableResource> pResource, const ezEnum<spMapAccess>& eAccess, ezUInt32 uiSubResource)
{
  if (pResource == nullptr)
  {
    EZ_ASSERT_DEV(pResource != nullptr, "Trying to map a resource that was not registered in the resource manager of the device. If you have created this resource without the resource factory, you should register it yourself.");
    return g_InvalidMappedResource;
  }

  if (pResource->IsInstanceOf<spBuffer>())
  {
    const auto pBuffer = pResource.Downcast<spBuffer>();
    EZ_ASSERT_DEV(pBuffer->GetUsage().IsAnySet(spBufferUsage::Dynamic | spBufferUsage::Staging), "Buffers must have the Staging or Dynamic usage flag to be mapped.");
    EZ_ASSERT_DEV(uiSubResource == 0, "Buffers must have the Staging or Dynamic usage flag to be mapped.");
    EZ_ASSERT_DEV(!((eAccess == spMapAccess::Read || eAccess == spMapAccess::ReadWrite) && !pBuffer->GetUsage().IsSet(spBufferUsage::Staging)), "spMapAccess::Read and spMapAccess::ReadWrite can only be used on buffers created with spBufferUsage::Staging.");
    return MapInternal(pBuffer, eAccess);
  }

  if (pResource->IsInstanceOf<spTexture>())
  {
    const auto pTexture = pResource.Downcast<spTexture>();
    EZ_ASSERT_DEV(pTexture->GetUsage().IsSet(spTextureUsage::Staging) == 0, "Texture must have the spTextureUsage::Staging usage flag to be mapped.");
    EZ_ASSERT_DEV(uiSubResource < pTexture->GetArrayLayerCount() * pTexture->GetMipCount(), "The subresource index must be less than the number of subresources in the Texture being mapped.");
    return MapInternal(pTexture, eAccess, uiSubResource);
  }

  EZ_ASSERT_NOT_IMPLEMENTED;
  return g_InvalidMappedResource;
}

void spDevice::UnMap(ezSharedPtr<spMappableResource> pResource, ezUInt32 uiSubresource)
{
  if (pResource == nullptr)
  {
    EZ_ASSERT_DEV(pResource != nullptr, "Trying to unmap a resource that was not registered in the resource manager of the device. If you have created this resource without the resource factory, you should register it yourself.");
    return;
  }

  if (pResource->IsInstanceOf<spBuffer>())
  {
    const auto pBuffer = pResource.Downcast<spBuffer>();
    UnMapInternal(pBuffer);
    return;
  }

  if (pResource->IsInstanceOf<spTexture>())
  {
    const auto pTexture = pResource.Downcast<spTexture>();
    UnMapInternal(pTexture, uiSubresource);
    return;
  }

  EZ_ASSERT_NOT_IMPLEMENTED;
}

void spDevice::UpdateBuffer(ezSharedPtr<spBuffer> pBuffer, ezUInt32 uiOffset, const void* pSource, ezUInt32 uiSize)
{
  if (pBuffer == nullptr)
  {
    EZ_ASSERT_DEV(pBuffer != nullptr, "Trying to update a buffer that was not registered in the resource manager of the device. If you have created this resource without the resource factory, you should register it yourself.");
    return;
  }

  EZ_ASSERT_DEV(uiOffset + uiSize <= pBuffer->GetSize(), "The data size given to UpdateBuffer is too large. The given buffer can only hold {} total bytes. The requested update would require {} bytes.", pBuffer->GetSize(), uiOffset + uiSize);
  UpdateBufferInternal(pBuffer, uiOffset, pSource, uiSize);
}

void spDevice::BeginFrame()
{
  // If debugging is enabled, start the profiler
  if (IsDebugEnabled())
    m_pFrameProfiler->Begin();
}

void spDevice::EndFrame()
{
  // If debugging is enabled, stop the profiler
  if (IsDebugEnabled())
    m_pFrameProfiler->End();

  Present();

  m_uiFrameCounter++;
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Device);
