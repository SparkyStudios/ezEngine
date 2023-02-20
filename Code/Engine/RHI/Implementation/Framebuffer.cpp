#include <RHI/RHIPCH.h>

#include <Foundation/Configuration/Singleton.h>

#include <RHI/Device.h>
#include <RHI/Framebuffer.h>

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

spFramebuffer::spFramebuffer(spFramebufferDescription description)
  : m_Description(std::move(description))
{
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Framebuffer);
