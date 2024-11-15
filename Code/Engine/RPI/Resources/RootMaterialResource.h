// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <RPI/Assets/RootMaterial.h>

#include <Core/ResourceManager/Resource.h>

namespace RPI
{
  /// \brief Handle for a root material resource.
  typedef ezTypedResourceHandle<class spRootMaterialResource> spRootMaterialResourceHandle;

  /// \brief Root Material resource descriptor.
  ///
  /// This descriptor stores data for a root material resource (properties, specialization constants,
  /// metadata, etc.) that can be used to create material instances.
  class SP_RPI_DLL spRootMaterialResourceDescriptor
  {
    friend class spRootMaterialResource;

  public:
    spRootMaterialResourceDescriptor();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spRootMaterial& GetRootMaterial() const { return m_RootMaterial; }
    [[nodiscard]] EZ_ALWAYS_INLINE spRootMaterial& GetRootMaterial() { return m_RootMaterial; }

    EZ_ALWAYS_INLINE void SetMaterial(const spRootMaterial& material) { m_RootMaterial = material; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFileName);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFileName);

  private:
    spRootMaterial m_RootMaterial;
  };

  class SP_RPI_DLL spRootMaterialResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spRootMaterialResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spRootMaterialResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spRootMaterialResource, spRootMaterialResourceDescriptor);

  public:
    static ezTypeVersion GetResourceVersion();

    spRootMaterialResource();

    [[nodiscard]] EZ_ALWAYS_INLINE const spRootMaterialResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spRootMaterialResourceDescriptor m_Descriptor;
  };
} // namespace RPI
