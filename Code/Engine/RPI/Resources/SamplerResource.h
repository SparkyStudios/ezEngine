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

#include <RPI/RPIDLL.h>

#include <RPI/Assets/Sampler.h>

#include <Core/ResourceManager/Resource.h>

namespace RPI
{
  typedef ezTypedResourceHandle<class spSamplerResource> spSamplerResourceHandle;

  class SP_RPI_DLL spSamplerResourceDescriptor
  {
    friend class spSamplerResource;

    friend class spTextureResourceLoader;

  public:
    spSamplerResourceDescriptor();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spSampler& GetSampler() const { return m_Sampler; }

    EZ_ALWAYS_INLINE void SetSampler(const spSampler& sampler) { m_Sampler = sampler; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFile);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFile);

  private:
    spSampler m_Sampler;
  };

  class SP_RPI_DLL spSamplerResource final : public ezResource
  {
    friend class spTextureResourceLoader;

    EZ_ADD_DYNAMIC_REFLECTION(spSamplerResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spSamplerResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spSamplerResource, spSamplerResourceDescriptor);

  public:
    spSamplerResource();

    [[nodiscard]] EZ_ALWAYS_INLINE const spSamplerResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spSamplerResourceDescriptor m_Descriptor;
  };
} // namespace RPI
