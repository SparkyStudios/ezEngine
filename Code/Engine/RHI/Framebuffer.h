#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Output.h>
#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a single attachment for a \a spFramebuffer.
  struct spFramebufferAttachmentDescription : ezHashableStruct<spFramebufferAttachmentDescription>
  {
    /// \brief Constructs an empty spFramebufferAttachmentDescription.
    spFramebufferAttachmentDescription() = default;

    /// \brief Constructs a spFramebufferAttachmentDescription.
    /// \param hTarget The texture to render into.
    explicit spFramebufferAttachmentDescription(const spResourceHandle& hTarget)
      : ezHashableStruct()
      , m_hTarget(hTarget)
    {
    }

    /// \brief Constructs a spFramebufferAttachmentDescription.
    /// \param hTarget The texture to render into.
    /// \param uiArrayLayer The array layer from the target to render into.
    /// \param uiMipLevel The mip level from the target to render into.
    spFramebufferAttachmentDescription(const spResourceHandle& hTarget, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel = 0)
      : ezHashableStruct()
      , m_hTarget(hTarget)
      , m_uiArrayLayer(uiArrayLayer)
      , m_uiMipLevel(uiMipLevel)
    {
    }

    /// \brief Compares this \a spFramebufferDescription with an \a other description for equality.
    EZ_ALWAYS_INLINE bool operator==(const spFramebufferAttachmentDescription& other) const
    {
      return m_hTarget == other.m_hTarget && m_uiArrayLayer == other.m_uiArrayLayer && m_uiMipLevel == other.m_uiMipLevel;
    }

    /// \brief Compares this \a spFramebufferDescription with an \a other description for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spFramebufferAttachmentDescription& other) const
    {
      return !(*this == other);
    }

    /// \brief The target texture to render into. For color attachments, this resource must have been
    /// created with the spTextureUsage::RenderTarget flag. For depth attachments, this resource must
    /// have been created with the spTextureUsage::DepthStencil flag.
    spResourceHandle m_hTarget{};

    /// \brief The array layer from the target to render into. The value must be less than the number
    /// of array layers in the target.
    ezUInt32 m_uiArrayLayer{0};

    /// \brief The mip level from the target to render into. The value must be less than the number
    /// of mip levels in the target.
    ezUInt32 m_uiMipLevel{0};
  };

  /// \brief Describes a \a spFramebuffer, for creation using a \a spDeviceResourceFactory.
  struct spFramebufferDescription : ezHashableStruct<spFramebufferDescription>
  {
    /// \brief Constructs an empty spFramebufferDescription.
    spFramebufferDescription()
      : ezHashableStruct()
      , m_DepthTarget(spResourceHandle())
    {
    }

    /// \brief Constructs a \a spFramebufferDescription with a single color target.
    /// \param [in] hDepthTarget The depth texture to use with the spFramebuffer.
    /// \param [in] hColorTarget The color target of the spFramebuffer.
    spFramebufferDescription(const spResourceHandle& hDepthTarget, const spResourceHandle& hColorTarget)
      : ezHashableStruct()
      , m_DepthTarget(hDepthTarget)
    {
      m_ColorTargets.PushBack(spFramebufferAttachmentDescription(hColorTarget));
    }

    /// \brief Constructs a spFramebufferDescription.
    /// \param [in] hDepthTarget The depth texture to use with the spFramebuffer.
    /// \param [in] colorTargets An array of color targets. All of them must have been created with the spTextureUsage::RenderTarget usage flag.
    spFramebufferDescription(const spResourceHandle& hDepthTarget, const ezStaticArray<spResourceHandle, SP_RHI_MAX_COLOR_TARGETS>& colorTargets)
      : ezHashableStruct()
      , m_DepthTarget(hDepthTarget)
    {
      for (ezUInt32 i = 0, l = colorTargets.GetCount(); i < l; i++)
        m_ColorTargets[i] = spFramebufferAttachmentDescription(colorTargets[i]);
    }

    /// \brief Compares this \a spFramebufferDescription to the given \a other description for equality.
    EZ_ALWAYS_INLINE bool operator==(const spFramebufferDescription& other) const
    {
      return m_DepthTarget == other.m_DepthTarget && m_ColorTargets == other.m_ColorTargets;
    }

    /// \brief Compares this \a spFramebufferDescription to the given \a other description for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spFramebufferDescription& other) const
    {
      return !(*this == other);
    }

    /// \brief The depth texture, which must have been created with the spTextureUsage::DepthStencil usage flag.
    spFramebufferAttachmentDescription m_DepthTarget;

    /// \brief An array of color targets. All of them must have been created with the spTextureUsage::RenderTarget usage flag.
    /// May be empty.
    ezStaticArray<spFramebufferAttachmentDescription, SP_RHI_MAX_COLOR_TARGETS> m_ColorTargets;
  };

  /// \brief A single render target attached to a spFramebuffer.
  class SP_RHI_DLL spFramebufferAttachment
  {
  public:
    explicit spFramebufferAttachment(spFramebufferAttachmentDescription description);

    /// \brief Gets the render target texture.
    EZ_NODISCARD spResourceHandle GetTarget() const;

    /// \brief Gets the used array layer in the target.
    EZ_NODISCARD ezUInt32 GetArrayLayer() const;

    /// \brief Gets the used mip level in the target.
    EZ_NODISCARD ezUInt32 GetMipLevel() const;

  private:
    spFramebufferAttachmentDescription m_Description;
  };

  /// \brief Controls which color and depth textures are set as active render targets.
  class SP_RHI_DLL spFramebuffer : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spFramebuffer, spDeviceResource);

    friend class spDeviceResourceFactory;

  public:
    /// \brief Gets the handle to the depth target attachment associated to this framebuffer. May be an invalid
    /// handle if no depth target was attached.
    EZ_NODISCARD virtual spResourceHandle GetDepthTarget() const;

    /// \brief Gets the array of color targets attachment associated to this framebuffer.
    EZ_NODISCARD virtual ezStaticArray<spResourceHandle, SP_RHI_MAX_COLOR_TARGETS> GetColorTargets() const;

    /// \brief Gets the number of color targets attached to this framebuffer.
    EZ_NODISCARD EZ_ALWAYS_INLINE virtual ezUInt32 GetColorTargetCount() const { return m_Description.m_ColorTargets.GetCount(); }

    /// \brief Gets a spOutputDescription giving the formats of depth and color targets.
    EZ_NODISCARD virtual const spOutputDescription& GetOutputDescription() const = 0;

    /// \brief Gets the framebuffer width;
    EZ_NODISCARD virtual ezUInt32 GetWidth() const = 0;

    /// \brief Gets the framebuffer height;
    EZ_NODISCARD virtual ezUInt32 GetHeight() const = 0;

    /// \brief Sets the given color target at the given index in the framebuffer.
    /// \param [in] uiIndex The index of the color target. Must be greater than 0 and less than SP_RHI_MAX_COLOR_TARGETS.
    /// \param [in] target The color target to set.
    void virtual SetColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target) = 0;

    /// \brief Takes a snapshot of the framebuffer.
    /// \param [in] uiColorTargetIndex The index of the color target to capture.
    /// \param [in] uiArrayLayer The array layer to capture.
    /// \param [in] uiMipLevel The mip level to capture.
    /// \param [out] out_Pixels The output array in which store the captured pixels.
    void virtual Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels) = 0;

  protected:
    explicit spFramebuffer(spFramebufferDescription description);

    spFramebufferDescription m_Description;
  };
} // namespace RHI
