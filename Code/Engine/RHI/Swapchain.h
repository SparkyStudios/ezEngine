#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

/// \brief A platform-specific rendering surface.
class SP_RHI_DLL spRenderingSurface : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(spRenderingSurface, ezReflectedClass);

protected:
  spRenderingSurface() = default;
  ~spRenderingSurface() override = default;
};

struct spSwapchainDescription : public ezHashableStruct<spSwapchainDescription>
{
  spSwapchainDescription()
    : ezHashableStruct<spSwapchainDescription>()
  {
  }

  spSwapchainDescription(ezUInt32 uiWidth, ezUInt32 uiHeight, const ezEnum<spPixelFormat>& eDepthFormat = spPixelFormat::Default, bool bUseSrgb = false, bool bVSync = false)
    : ezHashableStruct<spSwapchainDescription>()
    , m_uiWidth(uiWidth)
    , m_uiHeight(uiHeight)
    , m_eDepthFormat(eDepthFormat)
    , m_bVSync(bVSync)
    , m_bUseSrgb(bUseSrgb)
  {
  }

  /// \brief The \see spRenderingSurface to use as the target of rendering operations.
  spRenderingSurface* m_pRenderingSurface{nullptr};

  /// \brief The swapchain width.
  ezUInt32 m_uiWidth{0};

  /// \brief The swapchain height.
  ezUInt32 m_uiHeight{0};

  /// \brief Enables the usage of a depth texture in the swapchain.
  /// \note When this value is true, a suitable value must be set to \see m_eDepthFormat.
  bool m_bUseDepthTexture{false};

  /// \brief The swapchain's depth target pixel format.
  ezEnum<spPixelFormat> m_eDepthFormat{spPixelFormat::Default};

  /// \brief If true, the swapchain will (try to) synchronize with the screen native vertical refresh rate.
  bool m_bVSync{false};

  /// \brief If true, the swapchain will render to an SRGB pixel format.
  bool m_bUseSrgb{false};
};

class SP_RHI_DLL spSwapchain : public spDeviceResource
{
  friend class spDeviceResourceFactory;

public:
  /// \brief The \see spFramebuffer representing the render target of this swapchain.
  EZ_NODISCARD virtual spResourceHandle GetFramebuffer() const = 0;

  /// \brief Sets if the swapchain uses vertical synchronization with the display refresh rate.
  /// \param bVSync If true, the swapchain will (try to) synchronize with the display refresh rate.
  virtual void SetVSync(bool bVSync) = 0;

  /// \brief Returns whether the swapchain has VSync enabled.
  EZ_NODISCARD virtual bool GetVSync() const = 0;

  /// \brief Resizes the \see spFramebuffer of this swapchain.
  /// \param [in] uiWidth The new swapchain width.
  /// \param [in] uiHeight The new swapchain height.
  virtual void Resize(ezUInt32 uiWidth, ezUInt32 uiHeight) = 0;

protected:
  spSwapchain(spSwapchainDescription description)
    : m_Description(std::move(description))
  {
  }

  spSwapchainDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spSwapchain);
