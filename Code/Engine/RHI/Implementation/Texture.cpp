#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Texture.h>

#pragma region spTextureViewDescription

spTextureViewDescription::spTextureViewDescription(const spTexture* pTexture)
  : ezHashableStruct<spTextureViewDescription>()
  , m_hTarget(pTexture->GetHandle())
{
}

spTextureViewDescription::spTextureViewDescription(const spTexture* pTexture, const ezEnum<spPixelFormat>& eFormat)
  : ezHashableStruct<spTextureViewDescription>()
  , m_hTarget(pTexture->GetHandle())
  , m_bOverridePixelFormat(true)
  , m_eFormat(eFormat)
{
}

#pragma endregion

#pragma region spTextureSamplerManager

ezSharedPtr<spTextureView> spTextureSamplerManager::GetTextureView(const spDevice* pDevice, ezSharedPtr<spShaderResource> pResource)
{
  if (pResource->IsInstanceOf<spTextureView>())
    return pResource.Downcast<spTextureView>();

  if (pResource->IsInstanceOf<spTexture>())
    return pDevice->GetResourceManager()->GetResource<spTextureView>(pDevice->GetTextureSamplerManager()->GetFullTextureView(pResource->GetHandle()));

  EZ_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Texture);
