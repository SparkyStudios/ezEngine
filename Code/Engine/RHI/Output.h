#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>

class spFramebuffer;

/// \brief Describes an individual output attachment and its format.
struct spOutputAttachmentDescription : public ezHashableStruct<spOutputAttachmentDescription>
{
  spOutputAttachmentDescription()
    : ezHashableStruct<spOutputAttachmentDescription>()
  {
  }

  spOutputAttachmentDescription(ezEnum<spPixelFormat> eFormat)
    : ezHashableStruct<spOutputAttachmentDescription>()
    , m_eFormat(eFormat)
  {
  }

  /// \brief Compares this \see spOutputAttachmentDescription with the \a other description for equality.
  EZ_ALWAYS_INLINE bool operator==(const spOutputAttachmentDescription& other) const
  {
    return m_eFormat == other.m_eFormat;
  }

  /// \brief Compares this \see spOutputAttachmentDescription with the \a other description for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spOutputAttachmentDescription& other) const
  {
    return !(*this == other);
  }

  /// \brief The format of the output attachment.
  ezEnum<spPixelFormat> m_eFormat;
};

/// \brief Describes a set of output attachments and their formats.
struct SP_RHI_DLL spOutputDescription : public ezHashableStruct<spOutputDescription>
{
  /// \brief Indicates whether to use the depth attachment.
  bool m_bUseDepthAttachment{false};

  /// \brief A description of the depth attachment. Must be set if the output is
  /// configured to use the depth attachment.
  spOutputAttachmentDescription m_DepthAttachment;

  /// \brief An array of attachment descriptions. One for each color attachment. May be empty.
  ezStaticArray<spOutputAttachmentDescription, SP_RHI_MAX_COLOR_TARGETS> m_ColorAttachments;

  /// \brief The number of samples to use for each output attachment.
  ezEnum<spTextureSampleCount> m_eSampleCount;

  /// \brief Creates an output attachment from the given spFramebuffer.
  /// \param [in] pFramebuffer A pointer to a spFramebuffer object to copy the output description from.
  static spOutputDescription CreateFromFramebuffer(const spFramebuffer* pFramebuffer);

  /// \brief Creates an output attachment from the given spFramebuffer.
  /// \param [in] hFramebuffer A handle to a spFramebuffer resource to copy the output description from.
  static spOutputDescription CreateFromFramebuffer(spResourceHandle hFramebuffer);
};
