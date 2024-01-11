#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Device.h>
#include <RHIMTL/Framebuffer.h>
#include <RHIMTL/Swapchain.h>
#include <RHIMTL/Texture.h>

namespace RHI
{
#pragma region spFramebufferMTLBase

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFramebufferMTLBase, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spFramebufferMTLBase::spFramebufferMTLBase(RHI::spDeviceMTL* pDevice, const RHI::spFramebufferDescription& description)
    : spFramebuffer(description)
  {
    m_pDevice = pDevice;
  }

  MTL::RenderPassDescriptor* spFramebufferMTLBase::GetRenderPassDescriptor()
  {
    if (m_bIsDirty || m_pDescriptor == nullptr)
      CreateResource();

    return m_pDescriptor;
  }

#pragma endregion

#pragma region spFramebufferMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFramebufferMTL, 1, ezRTTINoAllocator);
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spFramebufferMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_pDescriptor = nullptr;
    m_bIsResourceCreated = false;
    m_bReleased = true;
  }

  bool spFramebufferMTL::IsReleased() const
  {
    return m_pDescriptor == nullptr;
  }

  void spFramebufferMTL::CreateResource()
  {
    const spTextureMTL* pDimensionTexture = nullptr;
    ezUInt32 uiDimensionMipLevel = 0;

    m_pDescriptor = MTL::RenderPassDescriptor::alloc()->init();

    if (!m_Description.m_DepthTarget.m_hTarget.IsInvalidated())
    {
      auto pDepthTexture = m_pDevice->GetResourceManager()->GetResource<spTextureMTL>(m_Description.m_DepthTarget.m_hTarget);
      EZ_ASSERT_DEV(pDepthTexture != nullptr, "Unable to find a texture in the device resource manager. If you have created that resource yourself, make sure to register it in the manager.");
      pDepthTexture->EnsureResourceCreated();

      pDimensionTexture = pDepthTexture;
      uiDimensionMipLevel = m_Description.m_DepthTarget.m_uiMipLevel;

      MTL::RenderPassDepthAttachmentDescriptor* pDepthDescriptor = m_pDescriptor->depthAttachment();
      pDepthDescriptor->setLoadAction(MTL::LoadActionLoad);
      pDepthDescriptor->setStoreAction(MTL::StoreActionStore);
      pDepthDescriptor->setTexture(pDepthTexture->GetMTLTexture());
      pDepthDescriptor->setSlice(m_Description.m_DepthTarget.m_uiArrayLayer);
      pDepthDescriptor->setLevel(m_Description.m_DepthTarget.m_uiMipLevel);

      if (spPixelFormatHelper::IsStencilFormat(pDepthTexture->GetFormat()))
      {
        MTL::RenderPassStencilAttachmentDescriptor* pStencilDescriptor = m_pDescriptor->stencilAttachment();
        pStencilDescriptor->setLoadAction(MTL::LoadActionLoad);
        pStencilDescriptor->setStoreAction(MTL::StoreActionStore);
        pStencilDescriptor->setTexture(pDepthTexture->GetMTLTexture());
        pStencilDescriptor->setSlice(m_Description.m_DepthTarget.m_uiArrayLayer);
        pStencilDescriptor->setLevel(m_Description.m_DepthTarget.m_uiMipLevel);
      }
    }

    for (ezUInt32 i = 0, l = m_Description.m_ColorTargets.GetCount(); i < l; ++i)
    {
      const auto& target = m_Description.m_ColorTargets[i];

      if (i == 0)
      {
        auto pColorTexture = m_pDevice->GetResourceManager()->GetResource<spTextureMTL>(target.m_hTarget);
        EZ_ASSERT_DEV(pColorTexture != nullptr, "Unable to find a texture in the device resource manager. If you have created that texture yourself, make sure to register it in the manager.");
        pColorTexture->EnsureResourceCreated();

        pDimensionTexture = pColorTexture;
        uiDimensionMipLevel = m_Description.m_DepthTarget.m_uiMipLevel;
      }

      ApplyColorTarget(i, target);
    }

    if (pDimensionTexture != nullptr)
    {
      ezUInt32 uiDepth = 0;
      spTextureHelper::GetMipDimensions(pDimensionTexture, uiDimensionMipLevel, m_uiWidth, m_uiHeight, uiDepth);
    }

    m_bReleased = false;
    m_bIsResourceCreated = true;
  }

  void spFramebufferMTL::SetColorTarget(ezUInt32 uiIndex, const RHI::spFramebufferAttachmentDescription& target)
  {
    const ezUInt32 uiCount = m_Description.m_ColorTargets.GetCount();
    EZ_ASSERT_DEV(uiCount > uiIndex, "Invalid color target index. Valid indexes are between 0 and {}.", uiCount);

    EnsureResourceCreated();
    ApplyColorTarget(uiIndex, target);
  }

  void spFramebufferMTL::Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels)
  {
    const ezUInt32 uiCount = m_Description.m_ColorTargets.GetCount();
    EZ_ASSERT_DEV(uiCount > uiColorTargetIndex, "Invalid color target index. Valid indexes are between 0 and {}.", uiCount);
    EZ_ASSERT_NOT_IMPLEMENTED;
  }

  spFramebufferMTL::spFramebufferMTL(RHI::spDeviceMTL* pDevice, const RHI::spFramebufferDescription& description)
    : spFramebufferMTLBase(pDevice, description)
    , m_OutputDescription()
  {
    m_pDevice = pDevice;

    m_OutputDescription = spOutputDescription::CreateFromFramebuffer(this);
  }

  spFramebufferMTL::~spFramebufferMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  void spFramebufferMTL::ApplyColorTarget(ezUInt32 uiIndex, const RHI::spFramebufferAttachmentDescription& target)
  {
    const auto pColorTexture = m_pDevice->GetResourceManager()->GetResource<spTextureMTL>(target.m_hTarget);
    EZ_ASSERT_DEV(pColorTexture != nullptr, "Unable to find a texture in the device resource manager. If you have created that texture yourself, make sure to register it in the manager.");
    pColorTexture->EnsureResourceCreated();

    MTL::RenderPassColorAttachmentDescriptor* pColorDescriptor = m_pDescriptor->colorAttachments()->object(uiIndex);
    pColorDescriptor->setTexture(pColorTexture->GetMTLTexture());
    pColorDescriptor->setLoadAction(MTL::LoadActionLoad);
    pColorDescriptor->setSlice(target.m_uiArrayLayer);
    pColorDescriptor->setLevel(target.m_uiMipLevel);
  }

#pragma endregion

#pragma region spSwapchainFramebufferMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSwapchainFramebufferMTL, 1, ezRTTINoAllocator);
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spSwapchainFramebufferMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_pParentSwapchain.Clear();
    m_pDepthTexture.Clear();

    m_bIsResourceCreated = false;
  }

  bool spSwapchainFramebufferMTL::IsReleased() const
  {
    return !m_bIsResourceCreated;
  }

  void spSwapchainFramebufferMTL::CreateResource()
  {
    m_pDescriptor = MTL::RenderPassDescriptor::alloc()->init();

    auto color0 = m_pDescriptor->colorAttachments()->object(0);
    color0->setTexture(m_pParentSwapchain->GetDrawable()->texture());
    color0->setLoadAction(MTL::LoadActionLoad);

    if (!GetDepthTarget().IsInvalidated())
    {
      m_pDepthTexture->EnsureResourceCreated();

      auto depthAttachment = m_pDescriptor->depthAttachment();
      depthAttachment->setTexture(m_pDepthTexture->GetMTLTexture());
      depthAttachment->setLoadAction(MTL::LoadActionLoad);
    }

    m_bIsResourceCreated = true;
  }

  void spSwapchainFramebufferMTL::SetColorTarget(ezUInt32 uiIndex, const spFramebufferAttachmentDescription& target)
  {
    // noop
  }

  void spSwapchainFramebufferMTL::Snapshot(ezUInt32 uiColorTargetIndex, ezUInt32 uiArrayLayer, ezUInt32 uiMipLevel, ezByteArrayPtr& out_Pixels)
  {
    const ezUInt32 uiCount = m_Description.m_ColorTargets.GetCount();
    EZ_ASSERT_DEV(uiCount > uiColorTargetIndex, "Invalid color target index. Valid indexes are between 0 and {}.", uiCount);
    EZ_ASSERT_NOT_IMPLEMENTED;
  }

  bool spSwapchainFramebufferMTL::IsRenderable() const
  {
    return false;
  }

  spSwapchainFramebufferMTL::spSwapchainFramebufferMTL(spDeviceMTL* pDevice, spSwapchainMTL* pParentSwapchain, ezUInt32 uiWidth, ezUInt32 uiHeight, const ezEnum<spPixelFormat>& eColorPixelFormat, const ezEnum<spPixelFormat>& eDepthPixelFormat)
    : spFramebufferMTLBase(pDevice, {})
  {
    m_pDevice = pDevice;

    if (eDepthPixelFormat != spPixelFormat::Unknown)
    {
      m_eDepthStencilFormat = eDepthPixelFormat;
      m_OutputDescription.m_DepthAttachment = spOutputAttachmentDescription(eDepthPixelFormat);
      Resize(uiWidth, uiHeight);
    }

    spOutputAttachmentDescription colorAttachment0(eColorPixelFormat);

    m_OutputDescription.m_ColorAttachments.EnsureCount(1);
    m_OutputDescription.m_ColorAttachments[0] = colorAttachment0;

    m_pParentSwapchain = {pParentSwapchain, m_pDevice->GetAllocator()};
  }

  spSwapchainFramebufferMTL::~spSwapchainFramebufferMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  void spSwapchainFramebufferMTL::Resize(ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;

    if (m_eDepthStencilFormat != spPixelFormat::Unknown)
      CreateDepthTexture(uiWidth, uiHeight);
  }

  void spSwapchainFramebufferMTL::SetDrawableTexture(MTL::Texture* pTexture)
  {
    if (m_pDrawableTexture != nullptr && m_pDrawableTexture->GetMTLTexture() == pTexture)
      return;

    m_pDrawableTexture.Clear();

    spTextureDescription drawableTextureDescription;
    m_pDrawableTexture = spTextureMTL::FromNative(static_cast<spDeviceMTL*>(m_pDevice), pTexture, drawableTextureDescription);

    m_Description.m_ColorTargets.SetCount(1);
    m_Description.m_ColorTargets[0] = spFramebufferAttachmentDescription(m_pDrawableTexture->GetHandle());
  }

  void spSwapchainFramebufferMTL::CreateDepthTexture(ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    EZ_ASSERT_DEV(m_eDepthStencilFormat != spPixelFormat::Unknown, "Invalid depth stencil format.");
    m_pDepthTexture.Clear();

    m_pDepthTexture = m_pDevice->GetResourceFactory()->CreateTexture(spTextureDescription::Texture2D(uiWidth, uiHeight, 1, 1, m_eDepthStencilFormat, spTextureUsage::DepthStencil)).Downcast<spTextureMTL>();
    m_Description.m_DepthTarget = spFramebufferAttachmentDescription(m_pDepthTexture->GetHandle());
    m_bIsDirty = true;
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Framebuffer);
