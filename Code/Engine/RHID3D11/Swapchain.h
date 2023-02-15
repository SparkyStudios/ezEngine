#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Swapchain.h>

#include <Foundation/Containers/List.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
#  include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#  include <windows.ui.xaml.media.dxinterop.h>
#endif

class spCommandListD3D11;
class spDeviceD3D11;
class spFramebufferD3D11;
class spTextureD3D11;

class SP_RHID3D11_DLL spRenderingSurfaceWin32 final : public spRenderingSurface
{
public:
  spRenderingSurfaceWin32(HWND pHWND, HINSTANCE pInstance, bool bIsFullscreen = false)
    : m_pHWND(pHWND)
    , m_pInstance(pInstance)
    , m_bIsFullscreen(bIsFullscreen)
  {
  }

  EZ_NODISCARD EZ_ALWAYS_INLINE HWND GetMainWindowHandle() const { return m_pHWND; }
  EZ_NODISCARD EZ_ALWAYS_INLINE HINSTANCE GetInstance() const { return m_pInstance; }

  EZ_NODISCARD EZ_ALWAYS_INLINE bool IsFullscreen() const { return m_bIsFullscreen; }

private:
  HWND m_pHWND;
  HINSTANCE m_pInstance;

  bool m_bIsFullscreen{false};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spRenderingSurfaceWin32);

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
class SP_RHID3D11_DLL spRenderingSurfaceUWP final : public spRenderingSurface
{
public:
  spRenderingSurfaceUWP(void* pSwapchainPanelNative, float fLogicalDpi)
    : m_pSwapchainPanelNative(pSwapchainPanelNative)
    , m_fLogicalDpi(fLogicalDpi)
  {
  }

  EZ_NODISCARD EZ_ALWAYS_INLINE void* GetSwapchainPanelNative() const { return m_pSwapchainPanelNative; }
  EZ_NODISCARD EZ_ALWAYS_INLINE float GetLogicalDpi() const { return m_fLogicalDpi; }

private:
  void* m_pSwapchainPanelNative;
  float m_fLogicalDpi;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spRenderingSurfaceUWP);
#endif

class SP_RHID3D11_DLL spSwapchainD3D11 final : public spSwapchain
{
  friend class spDeviceResourceFactoryD3D11;
  friend class spCommandListD3D11;

public:
  // spDeviceResource
  void ReleaseResource() override;
  bool IsReleased() const override;
  void SetDebugName(ezStringView sDebugName) override;

  // spSwapchain

  EZ_NODISCARD ezSharedPtr<spFramebuffer> GetFramebuffer() const override;
  void SetVSync(bool bVSync) override;
  EZ_NODISCARD EZ_ALWAYS_INLINE bool GetVSync() const override { return m_bVSync; }
  void Resize(ezUInt32 uiWidth, ezUInt32 uiHeight) override;
  void Present() override;

  // spSwapchainD3D11

  EZ_NODISCARD EZ_ALWAYS_INLINE IDXGISwapChain* GetD3D11SwapChain() const { return m_pD3D11SwapChain; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetSyncInterval() const { return m_uiSyncInterval; }

  spSwapchainD3D11(spDeviceD3D11* pDevice, const spSwapchainDescription& description);
  ~spSwapchainD3D11() override;

private:
  void AddCommandListReference(spCommandListD3D11* pCL);
  void RemoveCommandListReference(spCommandListD3D11* pCL);

  ID3D11Device* m_pD3D11Device{nullptr};
  IDXGISwapChain* m_pD3D11SwapChain{nullptr};

  ezSharedPtr<spTextureD3D11> m_pDepthTexture{nullptr};
  ezSharedPtr<spTextureD3D11> m_pBackBufferTexture{nullptr};
  ezSharedPtr<spFramebufferD3D11> m_pFramebuffer{nullptr};

  float m_fPixelScale{1.0f};
  DXGI_FORMAT m_eColorFormat{DXGI_FORMAT_UNKNOWN};

  bool m_bVSync{false};
  ezUInt32 m_uiSyncInterval{0};

  ezMutex m_CLsLock;
  ezList<spCommandListD3D11*> m_DependentCommandLists;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spSwapchainD3D11);
