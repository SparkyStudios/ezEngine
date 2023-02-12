#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>

#include <RHID3D11/Buffer.h>
#include <RHID3D11/CommandList.h>
#include <RHID3D11/Core.h>
#include <RHID3D11/Fence.h>
#include <RHID3D11/Profiler.h>
#include <RHID3D11/ResourceFactory.h>
#include <RHID3D11/ResourceManager.h>
#include <RHID3D11/Swapchain.h>
#include <RHID3D11/Texture.h>

EZ_IMPLEMENT_SINGLETON(spDeviceD3D11);

static bool SdkLayersAvailable()
{
  const HRESULT hr = D3D11CreateDevice(
    nullptr,
    D3D_DRIVER_TYPE_NULL, // There is no need to create a real hardware device.
    0,
    D3D11_CREATE_DEVICE_DEBUG, // Check for the SDK layers.
    nullptr,                   // Any feature level will do.
    0,
    D3D11_SDK_VERSION, // Always set this to D3D11_SDK_VERSION for Windows Store apps.
    nullptr,           // No need to keep the D3D device reference.
    nullptr,           // No need to know the feature level.
    nullptr            // No need to keep the D3D device context reference.
  );

  return SUCCEEDED(hr);
}

static bool Check16BitsPrecisionSupport(ID3D11Device* pDevice)
{
  D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT data;
  pDevice->CheckFeatureSupport(D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT, &data, sizeof(D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT));
  return (data.PixelShaderMinPrecision & D3D11_SHADER_MIN_PRECISION_16_BIT) != 0 && (data.AllOtherShaderStagesMinPrecision & D3D11_SHADER_MIN_PRECISION_16_BIT) != 0;
}

static bool CheckDoublePrecisionSupport(ID3D11Device* pDevice)
{
  D3D11_FEATURE_DATA_DOUBLES data;
  pDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &data, sizeof(D3D11_FEATURE_DATA_DOUBLES));
  return data.DoublePrecisionFloatShaderOps == TRUE;
}

static bool CheckConservativeRasterizationSupport(ID3D11Device3* pDevice3)
{
  if (pDevice3 == nullptr)
    return false;

  bool bSupported = false;

  D3D11_FEATURE_DATA_D3D11_OPTIONS2 featureOpts2;
  if (SUCCEEDED(pDevice3->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &featureOpts2, sizeof(featureOpts2))))
  {
    bSupported = (featureOpts2.ConservativeRasterizationTier != D3D11_CONSERVATIVE_RASTERIZATION_NOT_SUPPORTED);
  }

  return bSupported;
}

spDevice::HardwareInfo spDeviceD3D11::GetHardwareInfo() const
{
  return m_HardwareInfo;
}

ezEnum<spGraphicsApi> spDeviceD3D11::GetAPI() const
{
  return spGraphicsApi::Direct3D11;
}

spDeviceResourceFactory* spDeviceD3D11::GetResourceFactory() const
{
  return m_pResourceFactory;
}

spTextureSamplerManager* spDeviceD3D11::GetTextureSamplerManager() const
{
  // TODO
  return nullptr;
}

ezUInt32 spDeviceD3D11::GetConstantBufferMinOffsetAlignment() const
{
  return 256;
}

ezUInt32 spDeviceD3D11::GetStructuredBufferMinOffsetAlignment() const
{
  return 16;
}

ezSharedPtr<spSwapchain> spDeviceD3D11::GetMainSwapchain() const
{
  return m_pMainSwapchain;
}

const spDeviceCapabilities& spDeviceD3D11::GetCapabilities() const
{
  return m_Capabilities;
}

void spDeviceD3D11::SubmitCommandList(const spResourceHandle& hCommandList, const spResourceHandle& hFence)
{
  if (const auto pCommandList = m_pResourceManager->GetResource<spCommandListD3D11>(hCommandList); pCommandList != nullptr)
  {
    EZ_LOCK(m_ImmediateContextMutex);

    if (pCommandList->GetD3D11CommandList() != nullptr) // Command list already submitted or has been reset
    {
      m_pD3D11DeviceContext->ExecuteCommandList(pCommandList->GetD3D11CommandList(), FALSE);
      pCommandList->OnComplete();
    }
  }

  if (const auto pFence = m_pResourceManager->GetResource<spFenceD3D11>(hFence); pFence != nullptr)
    pFence->Raise();
}

bool spDeviceD3D11::WaitForFence(const spResourceHandle& hFence, double uiNanosecondsTimeout)
{
  const auto pFence = m_pResourceManager->GetResource<spFenceD3D11>(hFence);
  if (pFence == nullptr)
    return false;

  return pFence->Wait(ezTime::Nanoseconds(uiNanosecondsTimeout));
}

bool spDeviceD3D11::WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitAll, double uiNanosecondsTimeout)
{
  EZ_ASSERT_NOT_IMPLEMENTED;
  return false;
}

void spDeviceD3D11::ResetFence(const spResourceHandle& hFence)
{
  const auto pFence = m_pResourceManager->GetResource<spFenceD3D11>(hFence);
  if (pFence == nullptr)
    return;

  return pFence->Reset();
}

void spDeviceD3D11::Present()
{
  if (m_pMainSwapchain)
  {
    m_pMainSwapchain->Present();
  }
}

ezEnum<spTextureSampleCount> spDeviceD3D11::GetTextureSampleCountLimit(const ezEnum<spPixelFormat>& eFormat, bool bIsDepthFormat)
{
  const DXGI_FORMAT format = spToD3D11(eFormat, bIsDepthFormat);

  if (CheckFormatMultisample(format, 32))
    return spTextureSampleCount::ThirtyTwoSamples;

  if (CheckFormatMultisample(format, 16))
    return spTextureSampleCount::SixteenSamples;

  if (CheckFormatMultisample(format, 8))
    return spTextureSampleCount::EightSamples;

  if (CheckFormatMultisample(format, 4))
    return spTextureSampleCount::FourSamples;

  if (CheckFormatMultisample(format, 2))
    return spTextureSampleCount::TwoSamples;

  return spTextureSampleCount::None;
}

void spDeviceD3D11::UpdateTexture(const spResourceHandle& hResource, const void* pData, ezUInt32 uiSize, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer)
{
  if (auto pTextureD3D11 = m_pResourceManager->GetResource<spTextureD3D11>(hResource); pTextureD3D11->GetUsage().IsSet(spTextureUsage::Staging))
  {
    const ezUInt32 uiSubresource = spTextureHelper::CalculateSubresource(pTextureD3D11, uiMipLevel, uiArrayLayer);
    const spMappedResource& mappedResource = MapInternal(pTextureD3D11, spMapAccess::Write, uiSubresource);

    const ezUInt32 uiDenseRowSize = spPixelFormatHelper::GetRowPitch(uiWidth, pTextureD3D11->GetFormat());
    const ezUInt32 uiDenseSliceSize = spPixelFormatHelper::GetDepthPitch(uiDenseRowSize, uiHeight, pTextureD3D11->GetFormat());

    spTextureHelper::CopyTextureRegion(pData, 0, 0, 0, uiDenseRowSize, uiDenseSliceSize, mappedResource.GetData(), uiX, uiY, uiZ, mappedResource.GetRowPitch(), mappedResource.GetDepthPitch(), uiWidth, uiHeight, uiDepth, pTextureD3D11->GetFormat());

    UnMapInternal(pTextureD3D11, uiSubresource);
  }
  else
  {
    const ezUInt32 uiSubresource = spTextureHelper::CalculateSubresource(pTextureD3D11, uiMipLevel, uiArrayLayer);

    D3D11_BOX box;
    box.left = uiX;
    box.right = uiX + uiWidth;
    box.top = uiY;
    box.bottom = uiY + uiHeight;
    box.front = uiZ;
    box.back = uiZ + uiDepth;

    const ezUInt32 uiSourceRowPitch = spPixelFormatHelper::GetRowPitch(uiWidth, pTextureD3D11->GetFormat());
    const ezUInt32 uiSourceDepthPitch = spPixelFormatHelper::GetDepthPitch(uiSourceRowPitch, uiHeight, pTextureD3D11->GetFormat());

    EZ_LOCK(m_ImmediateContextMutex);
    m_pD3D11DeviceContext->UpdateSubresource(pTextureD3D11->GetD3D11Texture(), uiSubresource, &box, pData, uiSourceRowPitch, uiSourceDepthPitch);
  }
}

void spDeviceD3D11::ResolveTexture(const spResourceHandle& hSource, const spResourceHandle& hDestination)
{
}

void spDeviceD3D11::Destroy()
{
  WaitForIdle();

  for (auto it = m_AvailableStagingBuffers.GetIterator(); it.IsValid(); it.Next())
    m_pResourceManager->ReleaseResource(*it);

  m_AvailableStagingBuffers.Clear();

  // This call should release the swapchain resource... If not, the swapchain was surely still referenced somewhere.
  // That means there are some memory leaks.
  m_pMainSwapchain.Clear();

  m_pFrameProfiler.Clear();

  SP_RHI_DX11_RELEASE(m_pD3D11DeviceContext);
  SP_RHI_DX11_RELEASE(m_pD3D11Device3);

  if (IsDebugEnabled())
  {
    if (m_pD3D11Device->Release() > 0)
    {
      spD3D11ScopedResource<ID3D11Debug> pDeviceDebug;
      const HRESULT res = m_pD3D11Device->QueryInterface(IID_ID3D11Debug, reinterpret_cast<void**>(&pDeviceDebug));
      EZ_HRESULT_TO_ASSERT(res);

      if (*pDeviceDebug != nullptr)
        pDeviceDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
    }
  }
  else
  {
    SP_RHI_DX11_RELEASE(m_pD3D11Device);
  }

  SP_RHI_DX11_RELEASE(m_pDXGIAdapter);
  SP_RHI_DX11_RELEASE(m_pDXGIDevice);
}

void spDeviceD3D11::WaitForIdleInternal()
{
  // noop
}

const spMappedResource& spDeviceD3D11::MapInternal(spBuffer* pBuffer, ezEnum<spMapAccess> eAccess)
{
  const spMappedResourceCacheKey key(pBuffer, 0);
  EZ_LOCK(m_MappedResourcesMutex);

  spMappedResource mappedResource;
  if (m_MappedResourcesCache.TryGetValue(key, mappedResource))
  {
    EZ_ASSERT_DEV(mappedResource.GetAccess() == eAccess, "The given resource is already mapped in a different access mode.");
  }
  else
  {
    const auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pBuffer);
    EZ_LOCK(m_ImmediateContextMutex);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    const HRESULT res = m_pD3D11DeviceContext->Map(pBufferD3D11->GetD3D11Buffer(), 0, spToD3D11(eAccess, pBufferD3D11->GetUsage().IsSet(spBufferUsage::Dynamic)), 0, &mappedSubresource);
    EZ_HRESULT_TO_ASSERT(res);

    mappedResource = spMappedResource(pBufferD3D11->GetHandle(), eAccess, mappedSubresource.pData, pBufferD3D11->GetSize());
  }

  EZ_IGNORE_UNUSED(mappedResource.AddRef());
  m_MappedResourcesCache[key] = mappedResource;

  return m_MappedResourcesCache[key];
}

const spMappedResource& spDeviceD3D11::MapInternal(spTexture* pTexture, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubresource)
{
  const spMappedResourceCacheKey key(pTexture, uiSubresource);
  EZ_LOCK(m_MappedResourcesMutex);

  spMappedResource mappedResource;
  if (m_MappedResourcesCache.TryGetValue(key, mappedResource))
  {
    EZ_ASSERT_DEV(mappedResource.GetAccess() == eAccess, "The given resource is already mapped in a different access mode.");
  }
  else
  {
    const auto* pTextureD3D11 = ezStaticCast<spTextureD3D11*>(pTexture);
    EZ_LOCK(m_ImmediateContextMutex);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    const HRESULT res = m_pD3D11DeviceContext->Map(pTextureD3D11->GetD3D11Texture(), uiSubresource, spToD3D11(eAccess, false), 0, &mappedSubresource);
    EZ_HRESULT_TO_ASSERT(res);

    mappedResource = spMappedResource(pTextureD3D11->GetHandle(), eAccess, mappedSubresource.pData, pTextureD3D11->GetHeight() * mappedSubresource.RowPitch, uiSubresource, mappedSubresource.RowPitch, mappedSubresource.DepthPitch);
  }

  EZ_IGNORE_UNUSED(mappedResource.AddRef());
  m_MappedResourcesCache[key] = mappedResource;

  return m_MappedResourcesCache[key];
}

void spDeviceD3D11::UnMapInternal(spBuffer* pBuffer)
{
  const spMappedResourceCacheKey key(pBuffer, 0);
  EZ_LOCK(m_MappedResourcesMutex);

  spMappedResource mappedResource;
  bool bDone = m_MappedResourcesCache.TryGetValue(key, mappedResource);
  EZ_ASSERT_DEV(bDone, "The given resource is not mapped.");

  if (mappedResource.ReleaseRef() == 0)
  {
    const auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pBuffer);
    EZ_LOCK(m_ImmediateContextMutex);

    m_pD3D11DeviceContext->Unmap(pBufferD3D11->GetD3D11Buffer(), 0);
  }

  bDone = m_MappedResourcesCache.Remove(key);
  EZ_ASSERT_DEV(bDone, "Unable to unmap the resource.");
}

void spDeviceD3D11::UnMapInternal(spTexture* pTexture, ezUInt32 uiSubresource)
{
  const spMappedResourceCacheKey key(pTexture, uiSubresource);
  EZ_LOCK(m_MappedResourcesMutex);

  spMappedResource mappedResource;
  bool bDone = m_MappedResourcesCache.TryGetValue(key, mappedResource);
  EZ_ASSERT_DEV(bDone, "The given resource is not mapped.");

  if (mappedResource.ReleaseRef() == 0)
  {
    const auto* pTextureD3D11 = ezStaticCast<spTextureD3D11*>(pTexture);
    EZ_LOCK(m_ImmediateContextMutex);

    m_pD3D11DeviceContext->Unmap(pTextureD3D11->GetD3D11Texture(), uiSubresource);
  }

  bDone = m_MappedResourcesCache.Remove(key);
  EZ_ASSERT_DEV(bDone, "Unable to unmap the resource.");
}

void spDeviceD3D11::UpdateBufferInternal(spBuffer* pBuffer, ezUInt32 uiOffset, const void* pData, ezUInt32 uiSize)
{
  if (uiSize == 0)
    return;

  auto* pBufferD3D11 = ezStaticCast<spBufferD3D11*>(pBuffer);
  pBufferD3D11->EnsureResourceCreated();

  const bool bIsDynamic = pBuffer->GetUsage().IsSet(spBufferUsage::Dynamic);
  const bool bIsStaging = pBuffer->GetUsage().IsSet(spBufferUsage::Staging);
  const bool bIsConstantBuffer = pBuffer->GetUsage().IsSet(spBufferUsage::ConstantBuffer);
  const bool bUpdateFullBuffer = uiOffset == 0 && uiSize == pBuffer->GetSize();
  const bool bUseUpdateSubresource = (!bIsDynamic && !bIsStaging) && (!bIsConstantBuffer || bUpdateFullBuffer);
  const bool bUseMap = (bIsDynamic && bUpdateFullBuffer) || bIsStaging;

  if (bUseUpdateSubresource)
  {
    D3D11_BOX box{uiOffset, 0, 0, uiSize + uiOffset, 1, 1};

    {
      EZ_LOCK(m_ImmediateContextMutex);
      m_pD3D11DeviceContext->UpdateSubresource(pBufferD3D11->GetD3D11Buffer(), 0, bIsConstantBuffer ? nullptr : &box, pData, 0, 0);
    }
  }
  else if (bUseMap)
  {
    const spMappedResource mappedResource = MapInternal(pBuffer, spMapAccess::Write);
    ezMemoryUtils::Copy(static_cast<ezUInt8*>(mappedResource.GetData()) + uiOffset, static_cast<const ezUInt8*>(pData), uiSize);
    UnMapInternal(pBuffer);
  }
  else
  {
    spBufferD3D11* pStaging = GetFreeStagingBuffer(uiSize);
    UpdateBufferInternal(pStaging, 0, pData, uiSize);

    D3D11_BOX box;
    box.left = 0;
    box.right = uiSize;
    box.top = 0;
    box.bottom = 1;
    box.front = 0;
    box.back = 1;

    {
      EZ_LOCK(m_ImmediateContextMutex);
      m_pD3D11DeviceContext->CopySubresourceRegion(pBufferD3D11->GetD3D11Buffer(), 0, uiOffset, 0, 0, pStaging->GetD3D11Buffer(), 0, &box);
    }

    {
      EZ_LOCK(m_StagingResourcesMutex);
      m_AvailableStagingBuffers.PushBack(pStaging);
    }
  }
}

spDeviceD3D11::spDeviceD3D11(ezAllocatorBase* pAllocator, const spDeviceDescriptionD3D11& deviceDescription)
  : spDevice(pAllocator, static_cast<spDeviceDescription>(deviceDescription))
  , m_SingletonRegistrar(this)
{
  ezUInt32 uiFlags = deviceDescription.m_uiCreationFlags;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (deviceDescription.m_bDebug)
    uiFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  // If debug flag is set but SDK layers aren't available we can't enable debug.
  if (0 != (uiFlags & D3D11_CREATE_DEVICE_DEBUG) && !SdkLayersAvailable())
    uiFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

  m_bIsDebugEnabled = (uiFlags & D3D11_CREATE_DEVICE_DEBUG) != 0;

  constexpr D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
  HRESULT res = D3D11CreateDevice(deviceDescription.m_pD3D11Adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, uiFlags, &featureLevels[0], 2, D3D11_SDK_VERSION, &m_pD3D11Device, (D3D_FEATURE_LEVEL*)&m_uiFeatureLevel, nullptr);

  if (FAILED(res))
  {
    ezLog::Error("Failed to create D3D11 device. Trying fallback device creation.");
    res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, uiFlags, &featureLevels[0], 2, D3D11_SDK_VERSION, &m_pD3D11Device, (D3D_FEATURE_LEVEL*)&m_uiFeatureLevel, nullptr);
    EZ_HRESULT_TO_ASSERT(res);
  }

  {
    if (SUCCEEDED(m_pD3D11Device->QueryInterface(IID_IDXGIDevice1, reinterpret_cast<void**>(&m_pDXGIDevice))))
    {
      if (FAILED(m_pDXGIDevice->SetMaximumFrameLatency(1)))
      {
        ezLog::Warning("Failed to set max frames latency");
      }

      // Store a pointer to the adapter
      // This is for the case of no preferred adapter, or fallback to warp.
      res = m_pDXGIDevice->GetAdapter(&m_pDXGIAdapter);
      EZ_HRESULT_TO_ASSERT(res);

      DXGI_ADAPTER_DESC desc;
      res = m_pDXGIAdapter->GetDesc(&desc);
      EZ_HRESULT_TO_ASSERT(res);

      m_HardwareInfo.m_sName = desc.Description;
      m_HardwareInfo.m_uiID = desc.DeviceId;
      m_HardwareInfo.m_uiVideoMemory = desc.DedicatedVideoMemory;
    }
  }

  switch (m_pD3D11Device->GetFeatureLevel())
  {
    case D3D_FEATURE_LEVEL_10_0:
      m_ApiVersion = {10, 0, 0, 0};
      break;

    case D3D_FEATURE_LEVEL_10_1:
      m_ApiVersion = {10, 1, 0, 0};
      break;

    case D3D_FEATURE_LEVEL_11_0:
      m_ApiVersion = {11, 0, 0, 0};
      break;

    case D3D_FEATURE_LEVEL_11_1:
      m_ApiVersion = {11, 1, 0, 0};
      break;

    case D3D_FEATURE_LEVEL_12_0:
      m_ApiVersion = {12, 0, 0, 0};
      break;

    case D3D_FEATURE_LEVEL_12_1:
      m_ApiVersion = {12, 1, 0, 0};
      break;

    case D3D_FEATURE_LEVEL_12_2:
      m_ApiVersion = {12, 2, 0, 0};
      break;

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  {
    if (FAILED(m_pD3D11Device->QueryInterface(IID_ID3D11Device3, reinterpret_cast<void**>(&m_pD3D11Device3))))
    {
      ezLog::Warning("Failed to get ID3D11Device3 from D3D11 device, some features won't be available.");
      m_pD3D11Device3 = nullptr;
    }
  }

  m_pResourceManager = EZ_DEFAULT_NEW(spDeviceResourceManagerD3D11, this);
  m_pResourceFactory = EZ_DEFAULT_NEW(spDeviceResourceFactoryD3D11, this);

  if (deviceDescription.m_bHasMainSwapchain)
  {
    m_pMainSwapchain = m_pResourceFactory->CreateSwapchain(deviceDescription.m_MainSwapchainDescription).Downcast<spSwapchainD3D11>();
  }

  {
    m_pD3D11Device->GetImmediateContext(&m_pD3D11DeviceContext);

    D3D11_FEATURE_DATA_THREADING support;
    res = m_pD3D11Device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &support, sizeof(D3D11_FEATURE_DATA_THREADING));

    if (FAILED(res))
    {
      m_bSupportsConcurrentResources = false;
      m_bSupportsCommandLists = false;
    }
    else
    {
      m_bSupportsConcurrentResources = support.DriverConcurrentCreates;
      m_bSupportsCommandLists = support.DriverCommandLists;
    }

    m_Capabilities.m_bBufferRangeBinding = m_pD3D11Device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_1;
    m_Capabilities.m_bCommandListDebugMarkers = m_pD3D11Device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_1;
    m_Capabilities.m_bComputeShader = true;
    m_Capabilities.m_bDepthClipDisable = true;
    m_Capabilities.m_bDrawBaseInstance = true;
    m_Capabilities.m_bDrawBaseVertex = true;
    m_Capabilities.m_bDrawIndirect = true;
    m_Capabilities.m_bDrawIndirectBaseInstance = true;
    m_Capabilities.m_bFillModeWireframe = true;
    m_Capabilities.m_bGeometryShader = true;
    m_Capabilities.m_bIndependentBlend = true;
    m_Capabilities.m_bIsClipSpaceYInverted = false;
    m_Capabilities.m_bIsDepthRangeZeroToOne = true;
    m_Capabilities.m_bIsUvOriginTopLeft = true;
    m_Capabilities.m_bMultipleViewport = true;
    m_Capabilities.m_bSamplerAnisotropy = true;
    m_Capabilities.m_bSamplerLodBias = true;
    m_Capabilities.m_bShader16BitsPrecision = Check16BitsPrecisionSupport(m_pD3D11Device);
    m_Capabilities.m_bShaderFloat64 = CheckDoublePrecisionSupport(m_pD3D11Device);
    m_Capabilities.m_bStructuredBuffers = true;
    m_Capabilities.m_bSubsetTextureView = true;
    m_Capabilities.m_bTessellationShader = true;
    m_Capabilities.m_bTexture1D = true;
    m_Capabilities.m_bConservaiveRasterization = CheckConservativeRasterizationSupport(m_pD3D11Device3);
    m_Capabilities.m_bSupportCommandLists = m_bSupportsCommandLists;
    m_Capabilities.m_bSupportConcurrentResources = m_bSupportsConcurrentResources;
  }

  m_pFrameProfiler = EZ_NEW(m_pAllocator, spFrameProfilerD3D11, this);
}

bool spDeviceD3D11::CheckFormatMultisample(DXGI_FORMAT format, ezUInt32 uiSampleCount) const
{
  UINT uiQualityLevels = 0;
  m_pD3D11Device->CheckMultisampleQualityLevels(format, uiSampleCount, &uiQualityLevels);

  return uiQualityLevels != 0;
}

spBufferD3D11* spDeviceD3D11::GetFreeStagingBuffer(ezUInt32 uiSize)
{
  {
    EZ_LOCK(m_StagingResourcesMutex);

    for (auto it = m_AvailableStagingBuffers.GetIterator(); it.IsValid(); it.Next())
    {
      if ((*it)->GetSize() >= uiSize)
      {
        m_AvailableStagingBuffers.Remove(it);
        return (*it);
      }
    }
  }

  return ezStaticCast<spBufferD3D11*>(GetResourceFactory()->CreateBuffer(spBufferDescription(uiSize, spBufferUsage::Staging)));
}

spDeviceD3D11::~spDeviceD3D11()
{
  m_pResourceManager->ReleaseResources();

  EZ_DEFAULT_DELETE(m_pResourceManager);
  EZ_DEFAULT_DELETE(m_pResourceFactory);
}

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Device);
