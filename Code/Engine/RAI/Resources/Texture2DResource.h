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

#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Texture.h>

#include <RHI/Texture.h>

namespace RAI
{
  typedef ezTypedResourceHandle<class spTexture2DResource> spTexture2DResourceHandle;

  /// \brief Describes the format of a texture asset.
  class SP_RAI_DLL spTextureResourceDescriptor
  {
    friend class spTexture2DResource;

    friend class spTextureResourceLoader;

  public:
    spTextureResourceDescriptor();

    void Clear();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spTexture& GetTexture() const { return m_Texture; }

    EZ_ALWAYS_INLINE void SetTexture(const spTexture& texture) { m_Texture = texture; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFile);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFile);

  private:
    /// \brief The texture asset stored in this resource.
    spTexture m_Texture;

    RHI::spTextureDescription m_RHITextureDescription;

    ezArrayPtr<ezByteBlobPtr> m_ImageData;
  };

  class SP_RAI_DLL spTexture2DResource : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spTexture2DResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spTexture2DResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spTexture2DResource, spTextureResourceDescriptor);

  public:
    spTexture2DResource();
    explicit spTexture2DResource(DoUpdate ResourceUpdateThread);

    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spTexture> GetRHITexture() const { return m_RHITexture[m_uiLoadedTextures - 1]; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spSampler> GetRHISampler() const { return m_RHISampler; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spTextureResourceDescriptor m_Descriptor;

    ezSharedPtr<RHI::spSampler> m_RHISampler{nullptr};
    ezSharedPtr<RHI::spTexture> m_RHITexture[2]{nullptr, nullptr};
    ezUInt32 m_uiGPUMemoryUsed[2]{0, 0};

    ezUInt8 m_uiLoadedMipLevel{0};
    ezUInt8 m_uiLoadedTextures{0};
  };
} // namespace RAI
