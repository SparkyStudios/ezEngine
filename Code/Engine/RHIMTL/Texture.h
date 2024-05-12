#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Texture.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spTextureMTL final : public spTexture, public spDeferredDeviceResource
  {
    friend class spTextureViewMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spTextureMTL, spTexture);

    // spDeviceResource

  public:
    void SetDebugName(ezStringView sDebugName) override;
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spDeferredDeviceResource

  public:
    void CreateResource() override;

    // spTextureMTL

  public:
    static ezSharedPtr<spTextureMTL> FromNative(spDeviceMTL* pDevice, MTL::Texture* pNative, spTextureDescription& out_Description);

    spTextureMTL(spDeviceMTL* pDevice, const spTextureDescription& description);
    ~spTextureMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::Texture* GetMTLTexture() const { return m_pTexture; }
    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::TextureType GetMTLTextureType() const { return m_eTextureType; }

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::Buffer* GetMTLStagingBuffer() const { return m_pStagingBuffer; }

    EZ_NODISCARD ezUInt32 GetSubresourceSize(ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer) const;
    void GetSubresourceLayout(ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer, ezUInt32& out_uiRowPitch, ezUInt32& out_uiDepthPitch) const;

  private:
    MTL::Device* m_pMTLDevice{nullptr};
    MTL::Texture* m_pTexture{nullptr};
    MTL::Buffer* m_pStagingBuffer{nullptr};

    MTL::PixelFormat m_eFormat;
    MTL::TextureType m_eTextureType;
    bool m_bFromNative{false};
  };

  class SP_RHIMTL_DLL spTextureViewMTL final : public spTextureView, public spDeferredDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spTextureViewMTL, spTextureView);

    // spDeviceResource

  public:
    void SetDebugName(ezStringView sDebugName) override;
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spDeferredDeviceResource

  public:
    void CreateResource() override;

    // spTextureViewMTL

  public:
    spTextureViewMTL(spDeviceMTL* pDevice, const spTextureViewDescription& description);
    ~spTextureViewMTL() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE MTL::Texture* GetMTLTargetTexture() const { return m_pTargetTexture; }

  private:
    MTL::Device* m_pMTLDevice{nullptr};
    MTL::Texture* m_pTargetTexture{nullptr};

    bool m_bHasTextureView{false};
  };

  class SP_RHIMTL_DLL spTextureSamplerManagerMTL final : public spTextureSamplerManager
  {
    // spTextureSamplerManager

  public:
    ezSharedPtr<spTextureView> GetFullTextureView(ezSharedPtr<spTexture> pTexture) override;

    // spTextureSamplerManagerD3D11

  public:
    explicit spTextureSamplerManagerMTL(spDeviceMTL* pDevice);
  };
} // namespace RHI
