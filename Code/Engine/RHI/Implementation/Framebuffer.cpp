#include <RHI/RHIPCH.h>

#include <Foundation/Configuration/Singleton.h>

#include <RHI/Device.h>
#include <RHI/Framebuffer.h>

#pragma region spFramebufferAttachment

spFramebufferAttachment::spFramebufferAttachment(spFramebufferAttachmentDescription description)
  : m_Description(std::move(description))
{
  const auto* pDevice = ezSingletonRegistry::GetSingletonInstance<spDevice>();
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(GetTarget()));
}

spFramebufferAttachment::~spFramebufferAttachment()
{
  const auto* pDevice = ezSingletonRegistry::GetSingletonInstance<spDevice>();
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(GetTarget()));
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

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Framebuffer);
