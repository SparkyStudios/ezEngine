// Copyright (c) 2023-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <RPI/RPIPCH.h>

#include <RPI/Resources/Loaders/TextureResourceLoader.h>
#include <RPI/Resources/Texture2DResource.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#include <ktx.h>

using namespace RPI;

static spTextureResourceLoader s_TextureResourceLoader;

ezCVarFloat cvar_Streaming_TextureLoadDelay("Streaming.TextureLoadDelay", 0.0f, ezCVarFlags::Save, "Artificial texture loading slowdown.");

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RPI, TextureResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezResourceManager::SetResourceTypeLoader<spTexture2DResource>(&s_TextureResourceLoader);
    // ezResourceManager::SetResourceTypeLoader<spTexture3DResource>(&s_TextureResourceLoader);
    // ezResourceManager::SetResourceTypeLoader<spTextureCubeResource>(&s_TextureResourceLoader);
    // ezResourceManager::SetResourceTypeLoader<spRenderTargetResource>(&s_TextureResourceLoader);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezResourceManager::SetResourceTypeLoader<spTexture2DResource>(nullptr);
    // ezResourceManager::SetResourceTypeLoader<spTexture3DResource>(nullptr);
    // ezResourceManager::SetResourceTypeLoader<spTextureCubeResource>(nullptr);
    // ezResourceManager::SetResourceTypeLoader<spRenderTargetResource>(nullptr);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

ezResourceLoadData spTextureResourceLoader::OpenDataStream(const ezResource* pResource)
{
  LoadedData* pData = EZ_DEFAULT_NEW(LoadedData);

  ezResourceLoadData res;

  // Solid Color Textures
  if (ezPathUtils::HasExtension(pResource->GetResourceID(), "color"))
  {
    ezStringBuilder sName = pResource->GetResourceID();
    sName.RemoveFileExtension();

    bool bValidColor = false;
    const ezColorGammaUB color = ezConversionUtils::GetColorByName(sName, &bValidColor);

    if (!bValidColor)
    {
      ezLog::Error("'{0}' is not a valid color name. Using 'RebeccaPurple' as fallback.", sName);
    }

    pData->m_bColorData = true;

    ktxTexture2* texture;
    ktxTextureCreateInfo createInfo;

    createInfo.glInternalformat = 0;
    createInfo.baseWidth = 4;
    createInfo.baseHeight = 4;
    createInfo.baseDepth = 1;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;
    createInfo.vkFormat = 43; // VK_FORMAT_R8G8B8A8_SRGB
    createInfo.numDimensions = 2;

    KTX_error_code result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);

    if (result != KTX_SUCCESS)
      return res;

    constexpr ezUInt32 kPixelCount = 4 * 4 * 4;

    auto* pPixels = EZ_DEFAULT_NEW_RAW_BUFFER(ezUInt8, kPixelCount);

    for (ezUInt32 px = 0; px < kPixelCount; px += 4)
    {
      pPixels[px + 0] = color.r;
      pPixels[px + 1] = color.g;
      pPixels[px + 2] = color.b;
      pPixels[px + 3] = color.a;
    }

    result = ktxTexture_SetImageFromMemory(ktxTexture(texture), 0, 0, 0, pPixels, kPixelCount);
    EZ_DEFAULT_DELETE_RAW_BUFFER(pPixels);

    if (result != KTX_SUCCESS)
      return res;

    pData->m_pImage = EZ_DEFAULT_NEW(spImage);
    pData->m_pSampler = EZ_DEFAULT_NEW(spSampler);

    pData->m_pImage->SetWidth(4);
    pData->m_pImage->SetHeight(4);
    pData->m_pImage->SetDepth(1);
    pData->m_pImage->SetMipCount(1);
    pData->m_pImage->SetArrayLayerCount(1);
    pData->m_pImage->SetPixelFormat(RHI::spPixelFormat::R8G8B8A8UNormSRgb);

    auto& buffer = pData->m_pImage->GetData();
    {
      ezMemoryStreamWriter w(&buffer);

      buffer.Reserve(ktxTexture_GetDataSize(ktxTexture(texture)));

      ktx_uint8_t* data = nullptr;
      ktx_size_t wroteSize = 0;

      result = ktxTexture_WriteToMemory(ktxTexture(texture), &data, &wroteSize);

      if (result != KTX_SUCCESS)
      {
        free(data);
        return res;
      }

      if (w.WriteBytes(data, wroteSize).Failed())
      {
        free(data);
        return res;
      }

      free(data);
      buffer.Compact();
    }

    if (pData->m_pImage->LoadImageData().Failed())
      return res;
  }
  else
  {
    ezFileReader file;
    if (file.Open(pResource->GetResourceID()).Failed())
      return res;

    const ezStringBuilder sAbsolutePath = file.GetFilePathAbsolute();
    res.m_sResourceDescription = file.GetFilePathRelative().GetView();

#if EZ_ENABLED(EZ_SUPPORTS_FILE_STATS)
    {
      ezFileStats stat;
      if (ezFileSystem::GetFileStats(pResource->GetResourceID(), stat).Succeeded())
      {
        res.m_LoadedFileModificationDate = stat.m_LastModificationTime;
      }
    }
#endif

    // TODO: Get SRGB info from the texture format definition file

    if (sAbsolutePath.HasExtension("spTexture2D") || sAbsolutePath.HasExtension("spTexture3D") || sAbsolutePath.HasExtension("spTextureCube") || sAbsolutePath.HasExtension("spRenderTarget") || sAbsolutePath.HasExtension("spLUT"))
    {
      if (LoadTextureFile(file, *pData).Failed())
        return res;
    }
    else
      return res;
  }

  ezMemoryStreamWriter w(&pData->m_Storage);

  WriteTextureLoadStream(w, *pData);

  res.m_pDataStream = &pData->m_Reader;
  res.m_pCustomLoaderData = pData;

  if (cvar_Streaming_TextureLoadDelay > 0)
  {
    ezThreadUtils::Sleep(ezTime::Seconds(cvar_Streaming_TextureLoadDelay));
  }

  return res;
}

void spTextureResourceLoader::CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData)
{
  auto* pData = static_cast<LoadedData*>(loaderData.m_pCustomLoaderData);

  if (pData->m_bColorData)
  {
    EZ_DEFAULT_DELETE(pData->m_pImage);
    EZ_DEFAULT_DELETE(pData->m_pSampler);
  }

  EZ_DEFAULT_DELETE(pData);
}

bool spTextureResourceLoader::IsResourceOutdated(const ezResource* pResource) const
{
  // Solid Color textures are never outdated
  if (ezPathUtils::HasExtension(pResource->GetResourceID(), "color"))
    return false;

  // Don't try to reload a file that cannot be found
  ezStringBuilder sAbs;
  if (ezFileSystem::ResolvePath(pResource->GetResourceID(), &sAbs, nullptr).Failed())
    return false;

#if EZ_ENABLED(EZ_SUPPORTS_FILE_STATS)

  if (pResource->GetLoadedFileModificationTime().IsValid())
  {
    ezFileStats stat;
    if (ezFileSystem::GetFileStats(pResource->GetResourceID(), stat).Failed())
      return false;

    return !stat.m_LastModificationTime.Compare(pResource->GetLoadedFileModificationTime(), ezTimestamp::CompareMode::FileTimeEqual);
  }

#endif

  return true;
}

ezResult spTextureResourceLoader::LoadTextureFile(ezStreamReader& inout_stream, LoadedData& ref_data)
{
  // Read the hash, ignore it
  ezAssetFileHeader assetHeader;
  EZ_SUCCEED_OR_RETURN(assetHeader.Read(inout_stream));

  spTextureResourceDescriptor texture;
  EZ_SUCCEED_OR_RETURN(texture.Load(inout_stream));

  const ezResourceLock imageResource(texture.m_Texture.m_hImage, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (!imageResource.IsValid())
    return EZ_FAILURE;

  const ezResourceLock samplerResource(texture.m_Texture.m_hSampler, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (!samplerResource.IsValid())
    return EZ_FAILURE;

  ref_data.m_pImage = &imageResource.GetPointerNonConst()->m_Descriptor.m_Image;
  ref_data.m_pSampler = &samplerResource.GetPointerNonConst()->m_Descriptor.m_Sampler;

  return EZ_SUCCESS;
}

void spTextureResourceLoader::WriteTextureLoadStream(ezStreamWriter& w, const LoadedData& data)
{
  w.WriteBytes(&data.m_pImage, sizeof(spImage*)).IgnoreResult();
  w.WriteBytes(&data.m_pSampler, sizeof(spSampler*)).IgnoreResult();
}

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_Loaders_TextureResourceLoader);
