#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Texture.h>

#pragma region spTextureViewDescription

spTextureViewDescription::spTextureViewDescription(const spTexture* pTexture)
  : ezHashableStruct<spTextureViewDescription>()
  , m_hTarget(pTexture->GetHandle())
{
}

spTextureViewDescription::spTextureViewDescription(const spTexture* pTexture, ezEnum<spPixelFormat> eFormat)
  : ezHashableStruct<spTextureViewDescription>()
  , m_hTarget(pTexture->GetHandle())
  , m_bOverridePixelFormat(true)
  , m_eFormat(eFormat)
{
}

#pragma endregion

#pragma region spTextureSamplerManager

spTextureView* spTextureSamplerManager::GetTextureView(spDevice* pDevice, spShaderResource* pResource)
{
  if (pResource->IsInstanceOf<spTextureView>())
    return static_cast<spTextureView*>(pResource);

  if (pResource->IsInstanceOf<spTexture>())
    return pDevice->GetResourceManager()->GetResource<spTextureView>(pDevice->GetTextureSamplerManager()->GetFullTextureView(pResource->GetHandle()));

  EZ_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Texture);
