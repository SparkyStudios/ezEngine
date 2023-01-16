#include <RHI/RHIPCH.h>

#include <RHI/Device.h>

#pragma region spDevice

void spDevice::SubmitCommandList(const spResourceHandle& hCommandList)
{
  SubmitCommandList(hCommandList, spResourceHandle());
}

void spDevice::SubmitCommandListAsync(const spResourceHandle& hCommandList)
{
  SubmitCommandList(hCommandList, spResourceHandle());
}

void spDevice::WaitForFence(const spResourceHandle& hFence)
{
  WaitForFence(hFence, 0xffffffffffffffffui64);
}

bool spDevice::WaitForFence(const spResourceHandle& hFence, const ezTime& timeout)
{
  return WaitForFence(hFence, static_cast<ezUInt64>(timeout.GetNanoseconds()));
}

void spDevice::WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitForAll)
{
  WaitForFences(fences, bWaitForAll, 0xffffffffffffffffui64);
}

bool spDevice::WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitAll, const ezTime& timeout)
{
  return WaitForFences(fences, bWaitAll, static_cast<ezUInt64>(timeout.GetNanoseconds()));
}

void spDevice::ResizeSwapchain(ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  spResourceHandle hSwapchain = GetMainSwapchain();

  if (hSwapchain.IsInvalidated())
    return;

  spSwapchain* pSwapchain = GetResourceManager()->GetResource<spSwapchain>(hSwapchain);
  pSwapchain->Resize(uiWidth, uiHeight);
}

void spDevice::WaitForIdle()
{
  WaitForIdleInternal();
  GetResourceManager()->ReleaseResources();
}

spMappedResource spDevice::Map(const spResourceHandle& hResource, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubresource)
{
  spDeviceResource* pResource = GetResourceManager()->GetResource(hResource);
  if (pResource == nullptr)
  {
    EZ_ASSERT_DEV(pResource != nullptr, "Trying to map a resource that was not registered in the resource manager of the device. If you have created this resource without the resource factory, you should register it yourself.");
    return {};
  }

  if (auto* pBuffer = ezDynamicCast<spBuffer*>(pResource); pBuffer != nullptr)
  {
    EZ_ASSERT_DEV(pBuffer->GetUsage().IsAnySet(spBufferUsage::Dynamic | spBufferUsage::Staging), "Buffers must have the Staging or Dynamic usage flag to be mapped.");
    EZ_ASSERT_DEV(uiSubresource == 0, "Buffers must have the Staging or Dynamic usage flag to be mapped.");
    EZ_ASSERT_DEV(!((eAccess == spMapAccess::Read || eAccess == spMapAccess::ReadWrite) && !pBuffer->GetUsage().IsSet(spBufferUsage::Staging)), "spMapAccess::Read and spMapAccess::ReadWrite can only be used on buffers created with spBufferUsage::Staging.");
    return MapInternal(pBuffer, eAccess);
  }

  if (auto* pTexture = ezDynamicCast<spTexture*>(pResource); pTexture != nullptr)
  {
    EZ_ASSERT_DEV(pTexture->GetUsage().IsSet(spTextureUsage::Staging) == 0, "Texture must have the spTextureUsage::Staging usage flag to be mapped.");
    EZ_ASSERT_DEV(uiSubresource < pTexture->GetArrayLayerCount() * pTexture->GetMipCount(), "The subresource index must be less than the number of subresources in the Texture being mapped.");
    return MapInternal(pTexture, eAccess, uiSubresource);
  }

  EZ_ASSERT_NOT_IMPLEMENTED;
  return {};
}

void spDevice::UnMap(const spResourceHandle& hResource, ezUInt32 uiSubresource)
{
  spDeviceResource* pResource = GetResourceManager()->GetResource(hResource);
  if (pResource == nullptr)
  {
    EZ_ASSERT_DEV(pResource != nullptr, "Trying to unmap a resource that was not registered in the resource manager of the device. If you have created this resource without the resource factory, you should register it yourself.");
    return;
  }

  if (auto* pBuffer = ezDynamicCast<spBuffer*>(pResource); pBuffer != nullptr)
  {
    UnMapInternal(pBuffer);
    return;
  }

  if (auto* pTexture = ezDynamicCast<spTexture*>(pResource); pTexture != nullptr)
  {
    UnMapInternal(pTexture, uiSubresource);
    return;
  }
}

void spDevice::UpdateBuffer(const spResourceHandle& hResource, ezUInt32 uiOffset, void* pSource, ezUInt32 uiSize)
{
  spBuffer* pBuffer = GetResourceManager()->GetResource<spBuffer>(hResource);
  if (pBuffer == nullptr)
  {
    EZ_ASSERT_DEV(pBuffer != nullptr, "Trying to update a buffer that was not registered in the resource manager of the device. If you have created this resource without the resource factory, you should register it yourself.");
    return;
  }

  EZ_ASSERT_DEV(uiOffset + uiSize <= pBuffer->GetSize(), "The data size given to UpdateBuffer is too large. The given buffer can only hold {} total bytes. The requested update would require {} bytes.", pBuffer->GetSize(), uiOffset + uiSize);
  UpdateBufferInternal(pBuffer, uiOffset, pSource, uiSize);
}

#pragma endregion
