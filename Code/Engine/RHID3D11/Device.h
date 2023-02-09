#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Device.h>

class spDeviceResourceManagerD3D11;
class spDeviceResourceFactoryD3D11;
class spSwapchainD3D11;
class spBufferD3D11;

struct SP_RHID3D11_DLL spDeviceDescriptionD3D11 : public spDeviceDescription
{
  IDXGIAdapter* m_pD3D11Adapter{nullptr};
  ezUInt32 m_uiCreationFlags{0};
};

class SP_RHID3D11_DLL spDeviceD3D11 final : public spDevice
{
  // spDevice

public:
  EZ_NODISCARD HardwareInfo GetHardwareInfo() const override;
  EZ_NODISCARD ezEnum<spGraphicsApi> GetAPI() const override;
  EZ_NODISCARD spDeviceResourceFactory* GetResourceFactory() const override;
  EZ_NODISCARD spTextureSamplerManager* GetTextureSamplerManager() const override;
  EZ_NODISCARD ezUInt32 GetConstantBufferMinOffsetAlignment() const override;
  EZ_NODISCARD ezUInt32 GetStructuredBufferMinOffsetAlignment() const override;
  EZ_NODISCARD spResourceHandle GetMainSwapchain() const override;
  EZ_NODISCARD const spDeviceCapabilities& GetCapabilities() const override;
  void SubmitCommandList(const spResourceHandle& hCommandList, const spResourceHandle& hFence) override;
  void SubmitCommandListAsync(const spResourceHandle& hCommandList, const spResourceHandle& hFence) override;
  bool WaitForFence(const spResourceHandle& hFence, double uiNanosecondsTimeout) override;
  bool WaitForFences(const ezList<spResourceHandle>& fences, bool bWaitAll, double uiNanosecondsTimeout) override;
  void ResetFence(const spResourceHandle& hFence) override;
  ezEnum<spTextureSampleCount> GetTextureSampleCountLimit(const ezEnum<spPixelFormat>& eFormat, bool bIsDepthFormat) override;
  void UpdateTexture(const spResourceHandle& hResource, const void* pData, ezUInt32 uiSize, ezUInt32 uiX, ezUInt32 uiY, ezUInt32 uiZ, ezUInt32 uiWidth, ezUInt32 uiHeight, ezUInt32 uiDepth, ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer) override;
  void ResolveTexture(const spResourceHandle& hSource, const spResourceHandle& hDestination) override;
  void Destroy() override;
  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsDebugEnabled() const override { return m_bIsDebugEnabled; }

protected:
  void WaitForIdleInternal() override;
  const spMappedResource& MapInternal(spBuffer* pBuffer, ezEnum<spMapAccess> eAccess) override;
  const spMappedResource& MapInternal(spTexture* pTexture, ezEnum<spMapAccess> eAccess, ezUInt32 uiSubresource) override;
  void UnMapInternal(spBuffer* pBuffer) override;
  void UnMapInternal(spTexture* pTexture, ezUInt32 uiSubresource) override;
  void UpdateBufferInternal(spBuffer* pBuffer, ezUInt32 uiOffset, const void* pData, ezUInt32 uiSize) override;

  // spDeviceD3D11

public:
  explicit spDeviceD3D11(const spDeviceDescriptionD3D11& deviceDescription);

  EZ_NODISCARD EZ_ALWAYS_INLINE spDeviceResourceManagerD3D11* GetD3D11ResourceManager() const { return ezStaticCast<spDeviceResourceManagerD3D11*>(m_pResourceManager); }

  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11Device* GetD3D11Device() const { return m_pD3D11Device; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11Device3* GetD3D11Device3() const { return m_pD3D11Device3; }
  EZ_NODISCARD EZ_ALWAYS_INLINE IDXGIAdapter* GetDXGIAdapter() const { return m_pDXGIAdapter; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11DeviceContext* GetD3D11DeviceContext() const { return m_pD3D11DeviceContext; }

private:
  bool CheckFormatMultisample(DXGI_FORMAT format, ezUInt32 uiSampleCount);
  spBufferD3D11* GetFreeStagingBuffer(ezUInt32 uiSize);

  ID3D11Device* m_pD3D11Device{nullptr};
  ID3D11Device3* m_pD3D11Device3{nullptr};
  IDXGIAdapter* m_pDXGIAdapter{nullptr};
  ID3D11DeviceContext* m_pD3D11DeviceContext{nullptr};
  ID3D11Debug* m_pD3D11Debug{nullptr};
  D3D_FEATURE_LEVEL m_uiFeatureLevel;

  spDeviceResourceFactoryD3D11* m_pResourceFactory{nullptr};
  spSwapchainD3D11* m_pMainSwapchain{nullptr};

  bool m_bIsDebugEnabled{false};

  bool m_bSupportsConcurrentResources{false};
  bool m_bSupportsCommandLists{false};

  ezMutex m_ImmediateContextMutex;
  ezMutex m_MappedResourcesMutex;
  ezMutex m_StagingResourcesMutex;
  ezMutex m_ResetEventsMutex;

  ezMap<spMappedResourceCacheKey, spMappedResource> m_MappedResourcesCache;
  ezList<spBufferD3D11*> m_AvailableStagingBuffers;

  HardwareInfo m_HardwareInfo;
  spDeviceCapabilities m_Capabilities;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spDeviceD3D11);
