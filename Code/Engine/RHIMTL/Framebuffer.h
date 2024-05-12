#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Framebuffer.h>

namespace RHI
{
  class spDeviceMTL;
  class spTextureMTL;
  class spSwapchainMTL;

  class SP_RHIMTL_DLL spFramebufferMTLBase : public spFramebuffer, public spDeferredDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spFramebufferMTLBase, spFramebuffer);

    // spFramebufferMTLBase

  public:
    EZ_NODISCARD MTL::RenderPassDescriptor* GetRenderPassDescriptor();

    spFramebufferMTLBase(spDeviceMTL* pDevice, const spFramebufferDescription& description);
    ~spFramebufferMTLBase() override = default;

    virtual bool IsRenderable() const = 0;

  protected:
    MTL::RenderPassDescriptor* m_pDescriptor{nullptr};
    bool m_bIsDirty{true};
  };

  class SP_RHIMTL_DLL spFramebufferMTL : public spFramebufferMTLBase
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spFramebufferMTL, spFramebufferMTLBase);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spDeferredDeviceResource

  public:
    void CreateResource() override;

    // spFramebuffer

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE const spOutputDescription& GetOutputDescription() const override { return m_OutputDescription; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetWidth() const override { return m_uiWidth; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetHeight() const override { return m_uiHeight; }
    void SetColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target) override;
    void Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels) override;

    // spFramebufferMTLBase

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsRenderable() const override { return true; }

    // spFramebufferMTL

  public:
    spFramebufferMTL(spDeviceMTL* pDevice, const spFramebufferDescription& description);
    ~spFramebufferMTL() override;

  private:
    void ApplyColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target);

    spOutputDescription m_OutputDescription;
    ezUInt32 m_uiWidth{0};
    ezUInt32 m_uiHeight{0};
  };

  class SP_RHIMTL_DLL spSwapchainFramebufferMTL final : public spFramebufferMTLBase
  {
    friend class spSwapchainMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spSwapchainFramebufferMTL, spFramebufferMTLBase);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spDeferredDeviceResource

  public:
    void CreateResource() override;

    // spFramebuffer

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE const spOutputDescription& GetOutputDescription() const override { return m_OutputDescription; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetWidth() const override { return m_uiWidth; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetHeight() const override { return m_uiHeight; }
    void SetColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target) override;
    void Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels) override;

    // spFramebufferMTLBase

  public:
    EZ_NODISCARD bool IsRenderable() const override;

    // spFramebufferMTL

  public:
    spSwapchainFramebufferMTL(spDeviceMTL* pDevice, spSwapchainMTL* pParentSwapchain, ezUInt32 uiWidth, ezUInt32 uiHeight, const ezEnum<spPixelFormat>& eColorPixelFormat, const ezEnum<spPixelFormat>& eDepthPixelFormat = spPixelFormat::Unknown);
    ~spSwapchainFramebufferMTL() override;

    void Resize(ezUInt32 uiWidth, ezUInt32 uiHeight);
    void SetDrawableTexture(MTL::Texture* pTexture);

    bool EnsureDrawable();

  private:
    void CreateDepthTexture(ezUInt32 uiWidth, ezUInt32 uiHeight);

    spOutputDescription m_OutputDescription;
    ezUInt32 m_uiWidth{0};
    ezUInt32 m_uiHeight{0};

    ezSharedPtr<spTextureMTL> m_pDrawableTexture{nullptr};
    ezSharedPtr<spTextureMTL> m_pDepthTexture{nullptr};
    ezSharedPtr<spSwapchainMTL> m_pParentSwapchain{nullptr};

    ezEnum<spPixelFormat> m_eDepthStencilFormat{spPixelFormat::Unknown};
  };
} // namespace RHI
