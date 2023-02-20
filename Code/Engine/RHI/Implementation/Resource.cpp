#include <RHI/RHIPCH.h>

#include <RHI/Buffer.h>
#include <RHI/Device.h>
#include <RHI/RenderTarget.h>
#include <RHI/Resource.h>

#include <Foundation/Logging/Log.h>

#pragma region spDeviceResource

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spDeviceResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

#pragma endregion

#pragma region spDeferredDeviceResource

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spDeferredDeviceResource, ezNoBase, 1, ezRTTINoAllocator)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

void spDeferredDeviceResource::EnsureResourceCreated()
{
  if (IsResourceCreated())
    return;

  CreateResource();
}

#pragma endregion

#pragma region spShaderResource

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

#pragma endregion

#pragma region spMappableResource

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMappableResource, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

#pragma endregion

#pragma region spMappedResource

spMappedResource::spMappedResource()
  : ezRefCounted()
  , m_hResource()
  , m_eAccess(spMapAccess::Read)
{
}

spMappedResource::spMappedResource(const spResourceHandle& hResource, const ezEnum<spMapAccess>& eAccess, void* pData, ezUInt32 uiSize, ezUInt32 uiSubResource, ezUInt32 uiRowPitch, ezUInt32 uiDepthPitch)
  : ezRefCounted()
  , m_hResource(hResource)
  , m_eAccess(eAccess)
  , m_pData(pData)
  , m_uiSize(uiSize)
  , m_uiSubResource(uiSubResource)
  , m_uiRowPitch(uiRowPitch)
  , m_uiDepthPitch(uiDepthPitch)
{
}

spMappedResource::spMappedResource(const spResourceHandle& hResource, const ezEnum<spMapAccess>& eAccess, const ezByteArrayPtr& data)
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

#pragma region spDeviceResourceFactory

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(spDeviceResourceFactory, ezNoBase, 1, ezRTTINoAllocator)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

ezSharedPtr<spRenderTarget> spDeviceResourceFactory::CreateRenderTarget(const spRenderTargetDescription& description)
{
  auto pRenderTarget = EZ_NEW(m_pDevice->GetAllocator(), spRenderTarget, m_pDevice, description);
  m_pDevice->GetResourceManager()->RegisterResource(pRenderTarget);

  return pRenderTarget;
}

ezSharedPtr<spRenderTarget> spDeviceResourceFactory::CreateRenderTarget(ezSharedPtr<spFramebuffer> pFramebuffer)
{
  auto pRenderTarget = EZ_NEW(m_pDevice->GetAllocator(), spRenderTarget, m_pDevice, pFramebuffer);
  m_pDevice->GetResourceManager()->RegisterResource(pRenderTarget);

  return pRenderTarget;
}

ezSharedPtr<spRenderTarget> spDeviceResourceFactory::CreateRenderTarget(ezSharedPtr<spTexture> pTexture)
{
  auto pRenderTarget = EZ_NEW(m_pDevice->GetAllocator(), spRenderTarget, m_pDevice, pTexture);
  m_pDevice->GetResourceManager()->RegisterResource(pRenderTarget);

  return pRenderTarget;
}

#pragma endregion

#pragma region spDeviceResourceManager

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spDeviceResourceManager, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spDeviceResourceManager::spDeviceResourceManager(spDevice* pDevice)
  : m_pDevice(pDevice)
  , m_pAllocator(pDevice->GetAllocator())
  , m_RegisteredResources(pDevice->GetAllocator())
{
}

spDeviceResourceManager::~spDeviceResourceManager()
{
  m_RegisteredResources.Clear();

  m_pDevice = nullptr;
  m_pAllocator = nullptr;
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

ezUInt32 spDeviceResourceManager::IncrementResourceRef(const spResourceHandle& hResource) const
{
  const spDeviceResource* pResource = GetResource(hResource);
  if (pResource == nullptr)
    return 0;

  return pResource->AddRef();
}

ezUInt32 spDeviceResourceManager::DecrementResourceRef(const spResourceHandle& hResource) const
{
  const spDeviceResource* pResource = GetResource(hResource);
  if (pResource == nullptr)
    return 0;

  return pResource->ReleaseRef();
}

#pragma endregion

#pragma region spDefaultDeviceResourceManager

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spDefaultDeviceResourceManager, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spDefaultDeviceResourceManager::spDefaultDeviceResourceManager(spDevice* pDevice)
  : spDeviceResourceManager(pDevice)
{
}

spDefaultDeviceResourceManager::~spDefaultDeviceResourceManager()
{
  ReleaseResources();

  {
    EZ_LOG_BLOCK("RHI resources leak report");

    if (m_RegisteredResources.IsEmpty())
      ezLog::Success("All RHI resources have been cleaned-up.");
    else
      ezLog::Warning("{0} RHI resources have not been cleaned-up. This may lead to memory leaks.", m_RegisteredResources.GetCount());
  }
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

void spDefaultDeviceResourceManager::ReleaseResource(const spResourceHandle& hResource)
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
  // ezSharedPtr handles the reference counting for us. So here we are releasing the resource
  // only if the reference count is 0.
  if (pResource->GetRefCount() == 0)
  {
    pResource->ReleaseResource();
    m_RegisteredResources.Remove(pResource->GetHandle().GetInternalID(), nullptr);
    pResource->GetHandle().Invalidate();
    // EZ_DELETE(m_pDevice->GetAllocator(), pResource); - Should be handled by ezSharedPtr
    pResource = nullptr;
  }
}

#pragma endregion

#pragma region spResourceHelper

spBufferRange* spResourceHelper::GetBufferRange(const spDevice* pDevice, spResourceHandle hResource, ezUInt32 uiOffset)
{
  return GetBufferRange(
    pDevice,
    pDevice->GetResourceManager()->GetResource<spShaderResource>(hResource),
    uiOffset);
}

spBufferRange* spResourceHelper::GetBufferRange(const spDevice* pDevice, spShaderResource* pResource, ezUInt32 uiOffset)
{
  if (pResource == nullptr)
    return nullptr;

  spBufferRange* pResult = nullptr;

  if (const auto* pBufferRange = ezDynamicCast<spBufferRange*>(pResource); pBufferRange != nullptr)
    pResult = pDevice->GetResourceFactory()->CreateBufferRange(spBufferRangeDescription(pBufferRange->GetBuffer()->GetHandle(), pBufferRange->GetOffset() + uiOffset, pBufferRange->GetSize()));
  else if (const auto* pBuffer = ezDynamicCast<spBuffer*>(pResource); pBuffer != nullptr)
    pResult = pDevice->GetResourceFactory()->CreateBufferRange(spBufferRangeDescription(pBuffer->GetHandle(), uiOffset, pBuffer->GetSize()));

  return pResult;
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Resource);
