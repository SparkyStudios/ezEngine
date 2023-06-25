#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Texture.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(RHI::spTextureSamplerManager, ezNoBase, 1, ezRTTINoAllocator)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace RHI
{
#pragma region spTexture

  // clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spTexture, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spTexture::spTexture(spTextureDescription description)
    : m_Description(std::move(description))
  {
  }

#pragma endregion

#pragma region spTextureView

  // clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spTextureView, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spTextureView::spTextureView(spTextureViewDescription description)
    : m_Description(std::move(description))
  {
  }

#pragma endregion

#pragma region spTextureSamplerManager

  spTextureSamplerManager::spTextureSamplerManager(spDevice* pDevice)
    : m_pDevice(pDevice)
  {
  }

  ezSharedPtr<spTextureView> spTextureSamplerManager::GetTextureView(const spDevice* pDevice, ezSharedPtr<spShaderResource> pResource)
  {
    if (pResource->IsInstanceOf<spTexture>())
      return pDevice->GetTextureSamplerManager()->GetFullTextureView(pResource.Downcast<spTexture>());

    if (pResource->IsInstanceOf<spTextureView>())
      return pResource.Downcast<spTextureView>();

    EZ_ASSERT_NOT_IMPLEMENTED;
    return nullptr;
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Texture);
