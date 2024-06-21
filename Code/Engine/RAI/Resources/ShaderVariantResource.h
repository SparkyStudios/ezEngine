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

#include <RAI/ShaderVariant.h>

#include <Core/ResourceManager/Resource.h>

namespace RAI
{
  /// \brief Handle for a shader variant resource.
  typedef ezTypedResourceHandle<class spShaderVariantResource> spShaderVariantResourceHandle;

  /// \brief A shader variant resource descriptor.
  ///
  /// This descriptor is used to store the needed information for a shader variant resource.
  class SP_RAI_DLL spShaderVariantResourceDescriptor
  {
    friend class spShaderVariantResource;

  public:
    spShaderVariantResourceDescriptor();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spShaderVariant& GetShaderVariant() const { return m_ShaderVariant; }
    [[nodiscard]] EZ_ALWAYS_INLINE spShaderVariant& GetShaderVariant() { return m_ShaderVariant; }

    EZ_ALWAYS_INLINE void SetShaderVariant(const spShaderVariant& shaderVariant) { m_ShaderVariant = shaderVariant; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFileName);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFileName);

  private:
    spShaderVariant m_ShaderVariant;
  };

  class SP_RAI_DLL spShaderVariantResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spShaderVariantResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spShaderVariantResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spShaderVariantResource, spShaderVariantResourceDescriptor);

  public:
    spShaderVariantResource();

    [[nodiscard]] EZ_ALWAYS_INLINE const spShaderVariantResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spShaderVariantResourceDescriptor m_Descriptor;
  };
} // namespace RAI
