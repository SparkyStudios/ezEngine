#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Resource.h>

#pragma region spDeferredDeviceResource

void spDeferredDeviceResource::EnsureResourceCreated()
{
  if (IsResourceCreated())
    return;

  CreateResource();
}

#pragma endregion

#pragma region spMappedResource

spMappedResource::spMappedResource()
  : m_hResource()
  , m_eAccess(spMapAccess::Read)
{
}

spMappedResource::spMappedResource(const spResourceHandle& hResource, ezEnum<spMapAccess> eAccess, void* pData, ezUInt32 uiSize, ezUInt32 uiSubResource, ezUInt32 uiRowPitch, ezUInt32 uiDepthPitch)
  : m_hResource(hResource)
  , m_eAccess(eAccess)
  , m_pData(pData)
  , m_uiSize(uiSize)
  , m_uiSubResource(uiSubResource)
  , m_uiRowPitch(uiRowPitch)
  , m_uiDepthPitch(uiDepthPitch)
{
}

spMappedResource::spMappedResource(const spResourceHandle& hResource, ezEnum<spMapAccess> eAccess, const ezByteArrayPtr& data)
  : spMappedResource(hResource, eAccess, data.GetPtr(), data.GetCount(), 0, 0, 0)
{
}

const spResourceHandle& spMappedResource::GetResource() const
{
  return m_hResource;
}

ezEnum<spMapAccess> spMappedResource::GetAccess() const
{
  return m_eAccess;
}

void* spMappedResource::GetData() const
{
  return m_pData;
}

ezUInt32 spMappedResource::GetSize() const
{
  return m_uiSize;
}

ezUInt32 spMappedResource::GetSubResource() const
{
  return m_uiSubResource;
}

ezUInt32 spMappedResource::GetRowPitch() const
{
  return m_uiRowPitch;
}

ezUInt32 spMappedResource::GetDepthPitch() const
{
  return m_uiDepthPitch;
}

#pragma endregion

#pragma region spDeviceResourceManager

spDeviceResourceManager::~spDeviceResourceManager()
{
  m_RegisteredResources.Clear();
}

spResourceHandle spDeviceResourceManager::RegisterResource(spDeviceResource* pResource)
{
  // Set resource handle
  return pResource->m_Handle = spResourceHandle(m_RegisteredResources.Insert(pResource));
}

spDeviceResource* spDeviceResourceManager::GetResource(const spResourceHandle& hResource) const
{
  if (hResource.IsInvalidated())
    return nullptr;

  spDeviceResource* pResource = nullptr;
  m_RegisteredResources.TryGetValue(hResource.GetInternalID(), pResource);

  return pResource;
}

ezUInt32 spDeviceResourceManager::IncrementResourceRef(spResourceHandle hResource) const
{
  spDeviceResource* pResource = GetResource(hResource);
  if (pResource == nullptr)
    return 0;

  return pResource->AddRef();
}

ezUInt32 spDeviceResourceManager::DecrementResourceRef(spResourceHandle hResource) const
{
  spDeviceResource* pResource = GetResource(hResource);
  if (pResource == nullptr)
    return 0;

  return pResource->ReleaseRef();
}

#pragma endregion

#pragma region spDefaultDeviceResourceManager

spDefaultDeviceResourceManager::~spDefaultDeviceResourceManager()
{
  m_ResourcesQueue.Clear();
}

void spDefaultDeviceResourceManager::EnqueueReleaseResource(const spResourceHandle& hResource)
{
  spDeviceResource* pResource = GetResource(hResource);
  if (pResource == nullptr)
    return;

  EnqueueReleaseResource(pResource);
}

void spDefaultDeviceResourceManager::EnqueueReleaseResource(spDeviceResource* pResource)
{
  m_ResourcesQueue.PushBack(pResource);
}

void spDefaultDeviceResourceManager::ReleaseResource(spResourceHandle hResource)
{
  spDeviceResource* pResource = GetResource(hResource);
  if (pResource == nullptr)
    return;

  ReleaseResource(pResource);
}

void spDefaultDeviceResourceManager::ReleaseResources()
{
  while (!m_ResourcesQueue.IsEmpty())
  {
    spDeviceResource* pResource = m_ResourcesQueue.PeekBack();
    m_ResourcesQueue.PopBack();

    ReleaseResource(pResource);
  }
}

void spDefaultDeviceResourceManager::ReleaseResource(spDeviceResource* pResource)
{
  if (pResource->ReleaseRef() == 0)
  {
    pResource->ReleaseResource();
    m_RegisteredResources.Remove(pResource->GetHandle().GetInternalID(), nullptr);
  }
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Resource);
