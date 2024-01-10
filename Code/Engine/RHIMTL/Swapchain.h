#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Swapchain.h>

namespace RHI
{
  class spCommandListMTL;
  class spDeviceMTL;
  class spSwapchainFramebufferMTL;
  class spTextureMTL;

  class SP_RHIMTL_DLL spRenderingSurfaceNSWindow final : public spRenderingSurface
  {
    EZ_ADD_DYNAMIC_REFLECTION(spRenderingSurfaceNSWindow, spRenderingSurface);

  public:
    explicit spRenderingSurfaceNSWindow(void* pWindow)
      : m_pWindow(pWindow)
    {
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE void* GetWindow() const { return m_pWindow; }

    EZ_NODISCARD CA::MetalLayer* GetMetalLayer() const;

    EZ_NODISCARD ezUInt32 GetWidth() const;
    EZ_NODISCARD ezUInt32 GetHeight() const;

  private:
    void* m_pWindow = nullptr;
  };

  class SP_RHIMTL_DLL spSwapchainMTL final : public spSwapchain
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spSwapchainMTL, spSwapchain);

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

    // spSwapchainMTL

    void GetNextDrawable();
    EZ_NODISCARD EZ_ALWAYS_INLINE CA::MetalDrawable* GetDrawable() const { return m_pMetalDrawable; }

    spSwapchainMTL(spDeviceMTL* pDevice, const spSwapchainDescription& description);
    ~spSwapchainMTL() override;

  private:
    MTL::Device* m_pMTLDevice{nullptr};

    CA::MetalLayer* m_pMetalLayer{nullptr};
    CA::MetalDrawable* m_pMetalDrawable{nullptr};

    ezSharedPtr<spSwapchainFramebufferMTL> m_pSwapchainFramebuffer;
    bool m_bVSync{false};
  };
} // namespace RHI
