#include <RHI/RHIPCH.h>

#include <Foundation/Configuration/Singleton.h>

#include <RHI/Device.h>
#include <RHI/Framebuffer.h>

namespace RHI
{
#pragma region spFramebufferAttachment

  spFramebufferAttachment::spFramebufferAttachment(spFramebufferAttachmentDescription description)
    : m_Description(std::move(description))
  {
  }

  spResourceHandle spFramebufferAttachment::GetTarget() const
  {
    return m_Description.m_hTarget;
  }

  ezUInt32 spFramebufferAttachment::GetArrayLayer() const
  {
    return m_Description.m_uiArrayLayer;
  }

  ezUInt32 spFramebufferAttachment::GetMipLevel() const
  {
    return m_Description.m_uiMipLevel;
  }

#pragma endregion

#pragma region spFramebuffer

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFramebuffer, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spResourceHandle spFramebuffer::GetDepthTarget() const
  {
    return m_Description.m_DepthTarget.m_hTarget;
  }

  ezStaticArray<spResourceHandle, SP_RHI_MAX_COLOR_TARGETS> spFramebuffer::GetColorTargets() const
  {
    ezStaticArray<spResourceHandle, SP_RHI_MAX_COLOR_TARGETS> targets;
    targets.EnsureCount(m_Description.m_ColorTargets.GetCount());

    ezUInt32 uiColorTargetIndex = 0;
    for (const auto& target : m_Description.m_ColorTargets)
    {
      targets[uiColorTargetIndex] = target.m_hTarget;
      ++uiColorTargetIndex;
    }

    return targets;
  }

  spFramebuffer::spFramebuffer(spFramebufferDescription description)
    : m_Description(std::move(description))
  {
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Framebuffer);
