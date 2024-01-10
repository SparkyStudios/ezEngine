#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Core.h>
#include <RHIMTL/Device.h>
#include <RHIMTL/Texture.h>

namespace RHI
{
#pragma region spPlaceholderTextureMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spPlaceholderTextureMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spPlaceholderTextureMTL::ReleaseResource()
  {
    m_bReleased = true;
  }

  void spPlaceholderTextureMTL::Resize(ezUInt32 uiWidth, ezUInt32 uiHeight)
  {
    m_Description.m_uiWidth = uiWidth;
    m_Description.m_uiHeight = uiHeight;
  }

  spPlaceholderTextureMTL::spPlaceholderTextureMTL(RHI::spDeviceMTL* pDevice, const RHI::spTextureDescription& description)
    : spTexture(description)
  {
    m_pDevice = pDevice;
    m_bReleased = false;
  }

  spPlaceholderTextureMTL::~spPlaceholderTextureMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion

#pragma region spTextureMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spTextureMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spTextureMTL::SetDebugName(ezStringView sDebugName)
  {
    spTexture::SetDebugName(sDebugName);

    if (IsReleased() || sDebugName.IsEmpty())
      return;

    const ezStringBuilder sName(sDebugName);

    NS::String* pString = NS::String::string(sName.GetData(), NS::ASCIIStringEncoding);
    m_pTexture->setLabel(pString);
    SP_RHI_MTL_RELEASE(pString);
  }

  void spTextureMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_MTL_RELEASE(m_pTexture);
    SP_RHI_MTL_RELEASE(m_pStagingBuffer);

    m_bIsResourceCreated = false;
    m_bFromNative = false;
  }

  bool spTextureMTL::IsReleased() const
  {
    return m_pTexture == nullptr;
  }

  void spTextureMTL::CreateResource()
  {
    if (!m_Description.m_eUsage.IsSet(spTextureUsage::Staging))
    {
      spScopedMTLResource pDescriptor(MTL::TextureDescriptor::alloc()->init());
      pDescriptor->setWidth(m_Description.m_uiWidth);
      pDescriptor->setHeight(m_Description.m_uiHeight);
      pDescriptor->setDepth(m_Description.m_uiDepth);
      pDescriptor->setMipmapLevelCount(m_Description.m_uiMipCount);
      pDescriptor->setArrayLength(m_Description.m_uiArrayLayers);
      pDescriptor->setSampleCount(m_Description.m_eSampleCount);
      pDescriptor->setTextureType(m_eTextureType);
      pDescriptor->setPixelFormat(m_eFormat);
      pDescriptor->setUsage(spToMTL(m_Description.m_eUsage));
      pDescriptor->setStorageMode(MTL::StorageModePrivate);

      m_pTexture = m_pMTLDevice->newTexture(*pDescriptor);
    }
    else
    {
      const ezUInt32 blockSize = spPixelFormatHelper::IsCompressedFormat(m_Description.m_eFormat) ? 4u : 1u;
      ezUInt32 totalStorageSize = 0;

      for (ezUInt32 level = 0; level < m_Description.m_uiMipCount; level++)
      {
        ezUInt32 levelWidth, levelHeight, levelDepth;
        spTextureHelper::GetMipDimensions(this, level, levelWidth, levelHeight, levelDepth);
        ezUInt32 storageWidth = ezMath::Max(levelWidth, blockSize);
        ezUInt32 storageHeight = ezMath::Max(levelHeight, blockSize);
        totalStorageSize += levelDepth * spPixelFormatHelper::GetDepthPitch(
                                           spPixelFormatHelper::GetRowPitch(storageWidth, m_Description.m_eFormat),
                                           storageHeight,
                                           m_Description.m_eFormat);
      }

      totalStorageSize *= m_Description.m_uiArrayLayers;

      m_pStagingBuffer = m_pMTLDevice->newBuffer(totalStorageSize, MTL::ResourceStorageModeShared);
    }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    SetDebugName(m_sDebugName);
#endif

    m_bIsResourceCreated = true;
  }

  ezSharedPtr<spTextureMTL> spTextureMTL::FromNative(spDeviceMTL* pDevice, MTL::Texture* pNative, spTextureDescription& description)
  {
    description.m_uiWidth = static_cast<ezUInt32>(pNative->width());
    description.m_uiHeight = static_cast<ezUInt32>(pNative->height());
    description.m_uiDepth = static_cast<ezUInt32>(pNative->depth());
    description.m_uiMipCount = static_cast<ezUInt32>(pNative->mipmapLevelCount());
    description.m_uiArrayLayers = static_cast<ezUInt32>(pNative->arrayLength());
    description.m_eFormat = spFromMTL(pNative->pixelFormat());
    description.m_eSampleCount = spTextureSampleCount::GetSampleCount(pNative->sampleCount());
    description.m_eDimension = spFromMTL(pNative->textureType());
    description.m_eUsage = spFromMTL(pNative->usage(), spIsDepthFormat(pNative->pixelFormat()));

    auto pResult = pDevice->GetResourceFactory()->CreateTexture(description).Downcast<spTextureMTL>();
    pResult->m_pDevice = pDevice;
    pResult->m_pMTLDevice = ezStaticCast<spDeviceMTL*>(pDevice)->GetMTLDevice();
    pResult->m_pTexture = pNative;
    pResult->m_eFormat = spToMTL(description.m_eFormat, description.m_eUsage.IsSet(spTextureUsage::DepthStencil));
    pResult->m_eTextureType = spToMTL(
      description.m_eDimension,
      description.m_uiArrayLayers,
      description.m_eSampleCount != spTextureSampleCount::None,
      description.m_eUsage.IsSet(spTextureUsage::Cubemap));
    pResult->m_bFromNative = true;
    pResult->m_bIsResourceCreated = true;

    SP_RHI_MTL_RETAIN(pNative);

    return pResult;
  }

  spTextureMTL::spTextureMTL(spDeviceMTL* pDevice, const spTextureDescription& description)
    : spTexture(description)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();

    m_eFormat = spToMTL(description.m_eFormat, description.m_eUsage.IsSet(spTextureUsage::DepthStencil));
    m_eTextureType = spToMTL(
      description.m_eDimension,
      description.m_uiArrayLayers,
      description.m_eSampleCount != spTextureSampleCount::None,
      description.m_eUsage.IsSet(spTextureUsage::Cubemap));
  }

  spTextureMTL::~spTextureMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  ezUInt32 spTextureMTL::GetSubresourceSize(ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer) const
  {
    ezUInt32 uiWidth, uiHeight, uiDepth;
    spTextureHelper::GetMipDimensions(this, uiMipLevel, uiWidth, uiHeight, uiDepth);

    ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(GetFormat()) ? 4u : 1u;
    ezUInt32 uiStorageWidth = ezMath::Max(uiBlockSize, uiWidth);
    ezUInt32 uiStorageHeight = ezMath::Max(uiBlockSize, uiHeight);

    return uiDepth * spPixelFormatHelper::GetDepthPitch(
                       spPixelFormatHelper::GetRowPitch(uiStorageWidth, GetFormat()),
                       uiStorageHeight,
                       GetFormat());
  }

  void spTextureMTL::GetSubresourceLayout(ezUInt32 uiMipLevel, ezUInt32 uiArrayLayer, ezUInt32& out_uiRowPitch, ezUInt32& out_uiDepthPitch) const
  {
    ezUInt32 uiMipWidth, uiMipHeight, uiMipDepth;
    spTextureHelper::GetMipDimensions(this, uiMipLevel, uiMipWidth, uiMipHeight, uiMipDepth);

    ezUInt32 uiBlockSize = spPixelFormatHelper::IsCompressedFormat(GetFormat()) ? 4u : 1u;
    ezUInt32 uiStorageWidth = ezMath::Max(uiBlockSize, uiMipWidth);
    ezUInt32 uiStorageHeight = ezMath::Max(uiBlockSize, uiMipHeight);

    out_uiRowPitch = spPixelFormatHelper::GetRowPitch(uiStorageWidth, GetFormat());
    out_uiDepthPitch = spPixelFormatHelper::GetDepthPitch(out_uiRowPitch, uiStorageHeight, GetFormat());
  }

#pragma endregion

#pragma region spTextureViewMTL

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spTextureViewMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spTextureViewMTL::SetDebugName(ezStringView sDebugName)
  {
    spTextureView::SetDebugName(sDebugName);

    if (IsReleased() || sDebugName.IsEmpty())
      return;

    const ezStringBuilder sName(sDebugName);

    NS::String* pString = NS::String::string(sName.GetData(), NS::ASCIIStringEncoding);
    m_pTargetTexture->setLabel(pString);
    SP_RHI_MTL_RELEASE(pString);
  }

  void spTextureViewMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    if (m_bHasTextureView)
      SP_RHI_MTL_RELEASE(m_pTargetTexture);

    m_pTargetTexture = nullptr;

    m_bIsResourceCreated = false;
    m_bHasTextureView = false;
  }

  bool spTextureViewMTL::IsReleased() const
  {
    return m_pTargetTexture == nullptr;
  }

  void spTextureViewMTL::CreateResource()
  {
    const auto& pTexture = m_pDevice->GetResourceManager()->GetResource<spTextureMTL>(m_Description.m_hTarget);
    EZ_ASSERT_DEV(pTexture != nullptr, "Texture view resource using invalid texture resource as a target.");

    if (m_Description.m_uiBaseMipLevel != 0 || m_Description.m_uiMipCount != pTexture->GetMipCount() || m_Description.m_uiBaseArrayLayer != 0 || m_Description.m_uiArrayLayers != pTexture->GetArrayLayerCount() || m_Description.m_eFormat != pTexture->GetFormat())
    {
      m_bHasTextureView = true;
      const ezUInt32 effectiveArrayLayers = pTexture->GetUsage().IsSet(spTextureUsage::Cubemap) ? m_Description.m_uiArrayLayers * 6 : m_Description.m_uiArrayLayers;
      m_pTargetTexture = pTexture->GetMTLTexture()->newTextureView(
        spToMTL(m_Description.m_eFormat, pTexture->GetUsage().IsSet(spTextureUsage::DepthStencil)),
        pTexture->GetMTLTextureType(),
        NS::Range(m_Description.m_uiBaseMipLevel, m_Description.m_uiMipCount),
        NS::Range(m_Description.m_uiBaseArrayLayer, effectiveArrayLayers));
    }
    else
    {
      m_bHasTextureView = false;
      m_pTargetTexture = pTexture->GetMTLTexture();
    }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    SetDebugName(m_sDebugName);
#endif

    m_bIsResourceCreated = true;
  }

  spTextureViewMTL::spTextureViewMTL(spDeviceMTL* pDevice, const spTextureViewDescription& description)
    : spTextureView(description)
  {
    m_pDevice = pDevice;
    m_pMTLDevice = pDevice->GetMTLDevice();
  }

  spTextureViewMTL::~spTextureViewMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion

#pragma region spTextureSamplerManagerMTL

  ezSharedPtr<spTextureView> spTextureSamplerManagerMTL::GetFullTextureView(ezSharedPtr<spTexture> pTexture)
  {
    const auto* pDevice = ezSingletonRegistry::GetSingletonInstance<spDevice>();

    spTextureViewDescription description{};
    description.m_hTarget = pTexture->GetHandle();
    description.m_uiBaseMipLevel = 0;
    description.m_uiMipCount = pTexture->GetDescription().m_uiMipCount;
    description.m_uiBaseArrayLayer = 0;
    description.m_uiArrayLayers = pTexture->GetDescription().m_uiArrayLayers;
    description.m_eFormat = pTexture->GetDescription().m_eFormat;

    return pDevice->GetResourceFactory()->CreateTextureView(description);
  }

  spTextureSamplerManagerMTL::spTextureSamplerManagerMTL(spDeviceMTL* pDevice)
    : spTextureSamplerManager(pDevice)
  {
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Texture);
