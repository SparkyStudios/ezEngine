#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Texture.h>

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

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Texture);
