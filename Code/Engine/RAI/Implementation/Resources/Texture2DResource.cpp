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

#include <RAI/RAIPCH.h>

#include <RAI/Resources/Texture2DResource.h>

#include <RHI/Device.h>

#include <Foundation/Configuration/CVar.h>

ezCVarBool cvar_AlwaysLoadFullQualityTextures("r.AlwaysLoadFullQualityTextures", true, ezCVarFlags::Save, "Defines if textures should always be loaded with the best available quality.");

namespace RAI
{
  static constexpr ezTypeVersion kTexture2DResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spTexture2DResource, kTexture2DResourceVersion, ezRTTIDefaultAllocator<spTexture2DResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spTexture2DResource);
  // clang-format on

#pragma region spTexture2DResourceDescriptor

  spTextureResourceDescriptor::spTextureResourceDescriptor()
  {
    Clear();
  }

  void spTextureResourceDescriptor::Clear()
  {
    m_Texture.m_hImage.Invalidate();
    m_Texture.m_hSampler.Invalidate();
  }

  ezResult spTextureResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream << kTexture2DResourceVersion;

    inout_stream << m_Texture.m_hImage;
    inout_stream << m_Texture.m_hSampler;

    return EZ_SUCCESS;
  }

  ezResult spTextureResourceDescriptor::Save(ezStringView sFile)
  {
    EZ_LOG_BLOCK("spTextureResourceDescriptor::Save", sFile);

    ezFileWriter file;
    if (file.Open(sFile, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open texture file '{0}'", sFile);
      return EZ_FAILURE;
    }

    return Save(file);
  }

  ezResult spTextureResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kTexture2DResourceVersion);

    inout_stream >> m_Texture.m_hImage;
    inout_stream >> m_Texture.m_hSampler;

    return EZ_SUCCESS;
  }

  ezResult spTextureResourceDescriptor::Load(ezStringView sFile)
  {
    EZ_LOG_BLOCK("spTextureResourceDescriptor::Load", sFile);

    ezFileReader file;
    if (file.Open(sFile, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open texture file '{0}'", sFile);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spTexture2DResource

  spTexture2DResource::spTexture2DResource()
    : ezResource(DoUpdate::OnAnyThread, cvar_AlwaysLoadFullQualityTextures ? 1 : 2)
  {
  }

  spTexture2DResource::spTexture2DResource(ezResource::DoUpdate ResourceUpdateThread)
    : ezResource(ResourceUpdateThread, cvar_AlwaysLoadFullQualityTextures ? 1 : 2)
  {
  }

  ezResourceLoadDesc spTexture2DResource::UnloadData(ezResource::Unload WhatToUnload)
  {
    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    if (m_uiLoadedTextures > 0)
    {
      for (ezInt32 i = 0; i < 2; ++i)
      {
        --m_uiLoadedTextures;

        if (m_RHITexture[m_uiLoadedTextures]->IsReferenced())
        {
          m_RHITexture[m_uiLoadedTextures].Clear();
        }

        m_uiGPUMemoryUsed[m_uiLoadedTextures] = 0;

        if (WhatToUnload == Unload::OneQualityLevel || m_uiLoadedTextures == 0)
          break;
      }
    }

    if (WhatToUnload == Unload::AllQualityLevels && m_RHISampler->IsReferenced())
    {
      m_RHISampler.Clear();
    }

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsLoadable = m_uiLoadedTextures;
    res.m_uiQualityLevelsDiscardable = 2 - m_uiLoadedTextures;
    res.m_State = m_uiLoadedTextures == 0 ? ezResourceState::Unloaded : ezResourceState::Loaded;

    return res;
  }

  ezResourceLoadDesc spTexture2DResource::UpdateContent(ezStreamReader* pStream)
  {
    spTextureResourceDescriptor desc;

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;

    if (pStream == nullptr)
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    // skip the absolute file path data that the standard file reader writes into the stream
    {
      ezStringBuilder sAbsFilePath;
      *pStream >> sAbsFilePath;
    }

    ezAssetFileHeader AssetHash;
    AssetHash.Read(*pStream).IgnoreResult();

    if (desc.Load(*pStream).Failed())
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
    }

    return CreateResource(std::move(desc));
  }

  void spTexture2DResource::UpdateMemoryUsage(ezResource::MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spTexture2DResource);
    out_NewMemoryUsage.m_uiMemoryGPU = m_uiGPUMemoryUsed[0] + m_uiGPUMemoryUsed[1];
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spTexture2DResource, spTextureResourceDescriptor)
  {
    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = m_uiLoadedTextures;
    res.m_uiQualityLevelsLoadable = (cvar_AlwaysLoadFullQualityTextures ? 2 : 1) - m_uiLoadedTextures;
    res.m_State = ezResourceState::Loaded;

    const ezResourceLock imageResource(descriptor.m_Texture.GetImage(), ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    EZ_ASSERT_DEV(imageResource.IsValid(), "Unable to get the image resource from the texture.");

    const ezResourceLock samplerResource(descriptor.m_Texture.GetSampler(), ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    EZ_ASSERT_DEV(samplerResource.IsValid(), "Unable to get the sampler resource from the texture.");

    // Retrieve image and sampler RAI assets
    const auto& image = imageResource.GetPointer()->GetDescriptor().GetImage();
    const auto& sampler = samplerResource.GetPointer()->GetDescriptor().GetSampler();

    // Fill the RHI texture descriptor
    descriptor.m_Texture.SetDescription(RHI::spTextureDescription::Texture2D(
      image.GetWidth(),
      image.GetHeight(),
      image.GetMipCount(),
      image.GetArrayLayerCount(),
      image.GetPixelFormat(),
      RHI::spTextureUsage::Sampled));

    m_Descriptor = descriptor;

    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();

    // Set the correct texture description for the loaded mipmap
    RHI::spTextureDescription desc = descriptor.m_Texture.GetDescription();
    desc.m_uiWidth = RHI::spTextureHelper::GetMipDimension(desc.m_uiWidth, m_uiLoadedMipLevel);
    desc.m_uiHeight = RHI::spTextureHelper::GetMipDimension(desc.m_uiHeight, m_uiLoadedMipLevel);

    // Create the RHI texture
    m_RHITexture[m_uiLoadedTextures] = pDevice->GetResourceFactory()->CreateTexture(desc);

    // Fill the RHI texture with the image data starting from the first loaded mip level
    for (ezUInt8 m = 0, l = image.GetMipCount() - m_uiLoadedMipLevel; m < l; ++m)
    {
      // TODO: Support array textures
      pDevice->UpdateTexture(
        m_RHITexture[m_uiLoadedTextures],
        image.GetImageData(m),
        0, 0, 0,
        RHI::spTextureHelper::GetMipDimension(desc.m_uiWidth, m),
        RHI::spTextureHelper::GetMipDimension(desc.m_uiHeight, m),
        1, m, 0);
    }

    m_RHITexture[m_uiLoadedTextures]->SetDebugName(GetResourceDescription());

    if (m_RHISampler != nullptr && m_RHISampler->IsReleased())
    {
      m_RHISampler.Clear();
    }

    m_RHISampler = pDevice->GetResourceFactory()->CreateSampler(sampler.GetDescription());
    m_RHISampler->SetDebugName(samplerResource.GetPointer()->GetResourceDescription());

    // Calculate the GPU memory used for the texture
    m_uiGPUMemoryUsed[m_uiLoadedTextures] = RHI::spPixelFormatHelper::GetDepthPitch(
      RHI::spPixelFormatHelper::GetRowPitch(desc.m_uiWidth, desc.m_eFormat),
      desc.m_uiHeight,
      desc.m_eFormat);

    ++m_uiLoadedTextures;

    return res;
  }

#pragma endregion
} // namespace RAI
