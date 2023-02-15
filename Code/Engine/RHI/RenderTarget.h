#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Framebuffer.h>
#include <RHI/Texture.h>

struct spRenderTargetDescription : public ezHashableStruct<spRenderTargetDescription>
{
  /// \brief The width of the render target, in pixels.
  ezUInt32 m_uiWidth{0};

  /// \brief The height of the render target, in pixels.
  ezUInt32 m_uiHeight{0};

  /// \brief The render target quality.
  ezEnum<spRenderTargetQuality> m_eQuality;

  /// \brief Whether the render target should generate mipmaps.
  bool m_bGenerateMipMaps{false};

  /// \brief The number of mip levels to generate. This value should be set when \a m_bGenerateMipMaps is true.
  ezUInt32 m_uiMipmapsCount{1};

  /// \brief The number of samples to use for multisampling.
  ezEnum<spTextureSampleCount> m_eSampleCount;

  /// \brief Compares this render target description to another for equality.
  EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const spRenderTargetDescription& rhs) const
  {
    return m_uiWidth == rhs.m_uiWidth && m_uiHeight == rhs.m_uiHeight && m_eQuality == rhs.m_eQuality && m_bGenerateMipMaps == rhs.m_bGenerateMipMaps && m_uiMipmapsCount == rhs.m_uiMipmapsCount && m_eSampleCount == rhs.m_eSampleCount;
  }

  /// \brief Compares this render target description to another for inequality.
  EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const spRenderTargetDescription& rhs) const { return !operator==(rhs); }
};

class SP_RHI_DLL spRenderTarget : public spDeviceResource
{
  friend class spDeviceResourceFactory;

  // spDeviceResource

public:
  void ReleaseResource() override;

  // spRenderTarget

public:
  /// \brief Gets the texture on which the render target will be rendered. This will be the framebuffer back texture.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spTexture> GetTexture() const { return m_pTexture; }

  /// \brief Gets the framebuffer on which the render target will be rendered.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spFramebuffer> GetFramebuffer() const { return m_pFramebuffer; }

private:
  spRenderTarget(spDevice* pDevice, ezSharedPtr<spTexture> pTexture);
  spRenderTarget(spDevice* pDevice, ezSharedPtr<spFramebuffer> pFramebuffer);

  spRenderTarget(spDevice* pDevice, const spRenderTargetDescription& description);

  void GenerateFramebuffer();

  ezSharedPtr<spTexture> m_pTexture{nullptr};
  ezSharedPtr<spFramebuffer> m_pFramebuffer{nullptr};

  spRenderTargetDescription m_Description;
};
