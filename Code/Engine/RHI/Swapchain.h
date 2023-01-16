#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

struct SP_RHI_DLL spSwapchainDescription : public ezHashableStruct<spSwapchainDescription>
{
  ezUInt32 m_uiWidth;
  ezUInt32 m_uiHeight;
  ezEnum<spPixelFormat> m_eDepthFormat;
  bool m_bVSync;
  bool m_bUseSrgb;
};

class SP_RHI_DLL spSwapchain : public spDeviceResource
{
public:
  /// \brief The \see spFramebuffer representing the render target of this swapchain.
  EZ_NODISCARD EZ_ALWAYS_INLINE const spResourceHandle& GetFramebuffer() const { return m_Framebuffer; }

  /// \brief Resizes the \see spFramebuffer of this swapchain.
  /// \param [in] uiWidth The new swapchain width.
  /// \param [in] uiHeight The new swapchain height.
  virtual void Resize(ezUInt32 uiWidth, ezUInt32 uiHeight) = 0;

protected:
  spResourceHandle m_Framebuffer;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spSwapchain);
