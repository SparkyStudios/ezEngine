#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Device.h>
#include <RHI/Framebuffer.h>

class spDeviceD3D11;
class spTextureD3D11;
class spSwapchainD3D11;

class SP_RHID3D11_DLL spFramebufferD3D11 : public spFramebuffer, public spDeferredDeviceResource
{
  friend class spDeviceResourceFactoryD3D11;
  friend class spSwapchainD3D11;

public:
  // spDeviceResource

  void SetDebugName(ezStringView sDebugName) override;
  void ReleaseResource() override;
  bool IsReleased() const override;

  // spDeferredDeviceResource

  void CreateResource() override;

  // spFramebuffer

  spResourceHandle GetDepthTarget() const override;
  ezStaticArray<spResourceHandle, SP_RHI_MAX_COLOR_TARGETS> GetColorTargets() const override;
  EZ_NODISCARD EZ_ALWAYS_INLINE const spOutputDescription& GetOutputDescription() const override { return m_OutputDescription; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetWidth() const override { return m_uiWidth; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetHeight() const override { return m_uiHeight; }
  void SetColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target) override;
  void Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels) override;

  // spFramebufferD3D11

  spFramebufferD3D11(spDeviceD3D11* pDevice, const spFramebufferDescription& description);
  ~spFramebufferD3D11() override;

  EZ_NODISCARD EZ_ALWAYS_INLINE ezArrayPtr<ID3D11RenderTargetView*> GetRenderTargetViews() { return m_ColorTargets.GetArrayPtr(); }
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11RenderTargetView* GetRenderTargetView(ezUInt32 uiSlot) const { return m_ColorTargets[uiSlot]; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthTarget; }

  EZ_NODISCARD ezSharedPtr<spTextureD3D11> GetColorTarget(ezUInt32 uiColorTargetIndex) const;
  EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spSwapchainD3D11> GetParentSwapchain() const { return {m_pParentSwapchain, m_pDevice->GetAllocator()}; }

private:
  void ApplyColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target);

  ID3D11Device* m_pD3D11Device{nullptr};

  ezStaticArray<ID3D11RenderTargetView*, SP_RHI_MAX_COLOR_TARGETS> m_ColorTargets;
  ID3D11DepthStencilView* m_pDepthTarget{nullptr};

  spSwapchainD3D11* m_pParentSwapchain{nullptr};

  spOutputDescription m_OutputDescription;
  ezUInt32 m_uiWidth{0};
  ezUInt32 m_uiHeight{0};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spFramebufferD3D11);
