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

#include <RPI/Assets/Material.h>

#include <Core/ResourceManager/Resource.h>

namespace RPI
{
  /// \brief Handle for a material resource.
  typedef ezTypedResourceHandle<class spMaterialResource> spMaterialResourceHandle;

  /// \brief Material resource descriptor.
  ///
  /// This descriptor stores data for a material resource (properties, material data, specialization constants.) that
  /// are used to generate the appropriate shader.
  class SP_RPI_DLL spMaterialResourceDescriptor
  {
    friend class spMaterialResource;

  public:
    spMaterialResourceDescriptor();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spMaterial& GetMaterial() const { return m_Material; }
    [[nodiscard]] EZ_ALWAYS_INLINE spMaterial& GetMaterial() { return m_Material; }

    EZ_ALWAYS_INLINE void SetMaterial(const spMaterial& material) { m_Material = material; }

    [[nodiscard]] EZ_ALWAYS_INLINE spRootMaterialResourceHandle GetRootMaterialResource() const { return m_Material.m_hRootMaterialResource; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFileName);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFileName);

  private:
    spMaterial m_Material;
  };

  /// \brief Material resource.
  ///
  /// This resource provides a material (properties, shader, and specialization constants.) that can be used by other resources.
  /// It is a simple wrapper around a material descriptor.
  ///
  /// The material resource is designed to be used in a resource pipeline (like the Render Pipeline Interface), where the material
  /// is loaded, compiled, and then cached for later use.
  class SP_RPI_DLL spMaterialResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spMaterialResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spMaterialResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spMaterialResource, spMaterialResourceDescriptor);

  public:
    static ezTypeVersion GetResourceVersion();

    spMaterialResource();

    [[nodiscard]] EZ_ALWAYS_INLINE const spMaterialResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spMaterialResourceDescriptor m_Descriptor;
  };
} // namespace RPI
