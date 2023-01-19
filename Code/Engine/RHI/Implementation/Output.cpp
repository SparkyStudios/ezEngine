#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Framebuffer.h>
#include <RHI/Output.h>

#pragma region spOutputDescription

spOutputDescription spOutputDescription::CreateFromFramebuffer(const spFramebuffer* pFramebuffer)
{
  ezEnum<spTextureSampleCount> eSampleCount = spTextureSampleCount::None;
  spOutputAttachmentDescription depthAttachment;
  bool bUseDepthAttachment = false;

  const auto* pDevice = ezSingletonRegistry::GetSingletonInstance<spDevice>();

  if (!pFramebuffer->GetDepthTarget().IsInvalidated())
  {
    const auto* pTexture = pDevice->GetResourceManager()->GetResource<spTexture>(pFramebuffer->GetDepthTarget());
    EZ_ASSERT_DEV(pTexture != nullptr, "Trying to get an unregistered texture from the device.");

    bUseDepthAttachment = true;
    depthAttachment = spOutputAttachmentDescription(pTexture->GetFormat());
    eSampleCount = pTexture->GetSampleCount();
  }

  auto colorTargets = pFramebuffer->GetColorTargets();

  ezStaticArray<spOutputAttachmentDescription, SP_RHI_MAX_COLOR_TARGETS> colorAttachments;
  colorAttachments.Reserve(colorTargets.GetCount());

  for (ezUInt32 i = 0, l = colorTargets.GetCount(); i < l; ++i)
  {
    const auto* pTexture = pDevice->GetResourceManager()->GetResource<spTexture>(colorTargets[i]);
    EZ_ASSERT_DEV(pTexture != nullptr, "Trying to get an unregistered texture from the device.");

    colorAttachments[i] = spOutputAttachmentDescription(pTexture->GetFormat());
    eSampleCount = pTexture->GetSampleCount();
  }

  return {
    {},
    bUseDepthAttachment,
    depthAttachment,
    colorAttachments,
    eSampleCount,
  };
}

spOutputDescription spOutputDescription::CreateFromFramebuffer(spResourceHandle hFramebuffer)
{
  spDevice* pDevice = ezSingletonRegistry::GetSingletonInstance<spDevice>();
  auto* pFramebuffer = pDevice->GetResourceManager()->GetResource<spFramebuffer>(hFramebuffer);
  EZ_ASSERT_DEV(pFramebuffer != nullptr, "Trying to get an unregistered framebuffer from the device.");

  return CreateFromFramebuffer(pFramebuffer);
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Output);
