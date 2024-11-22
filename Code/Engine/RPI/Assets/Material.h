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

#include <RPI/Materials/MaterialData.h>
#include <RPI/Resources/RootMaterialResource.h>

#include <RHI/Core.h>

namespace RPI
{
  class SP_RPI_DLL spMaterial
  {
    friend class spMaterialResource;
    friend class spMaterialResourceDescriptor;

  public:
    struct PropertyType
    {
      typedef ezUInt8 StorageType;

      enum Enum : StorageType
      {
        Boolean,
        Integer,
        UnsignedInteger,
        Float,
        Texture2D,

        Default = Boolean
      };
    };

    struct Property
    {
      ezEnum<PropertyType> m_eType{PropertyType::Default};
      ezHashedString m_sName;
      ezVariant m_Value;

      EZ_ALWAYS_INLINE bool operator==(const Property& rhs) const { return m_eType == rhs.m_eType && m_sName == rhs.m_sName && m_Value == rhs.m_Value; }
    };

    spMaterial();

    ~spMaterial();

    void Clear();

    [[nodiscard]] bool HasSpecializationConstant(const ezTempHashedString& name) const;
    void AddSpecializationConstant(const RHI::spShaderSpecializationConstant& constant);
    void RemoveSpecializationConstant(const ezTempHashedString& name);
    [[nodiscard]] const RHI::spShaderSpecializationConstant* GetSpecializationConstant(const ezTempHashedString& name) const;
    [[nodiscard]] EZ_ALWAYS_INLINE const ezDynamicArray<RHI::spShaderSpecializationConstant>& GetSpecializationConstants() const { return m_SpecializationConstants; }

    void SetProperty(const ezTempHashedString& name, const ezVariant& value);
    [[nodiscard]] ezVariant GetProperty(const ezTempHashedString& name) const;

    [[nodiscard]] EZ_ALWAYS_INLINE spMaterialData& GetData() { return m_Data; }
    [[nodiscard]] EZ_ALWAYS_INLINE const spMaterialData& GetData() const { return m_Data; }

    void SetRootMaterialResource(const spRootMaterialResourceHandle& hRootMaterialResource);
    [[nodiscard]] EZ_ALWAYS_INLINE spRootMaterialResourceHandle GetRootMaterialResource() const { return m_hRootMaterialResource; }

  private:
    [[nodiscard]] ezUInt64 GetHeapMemoryUsage() const;

    spRootMaterialResourceHandle m_hRootMaterialResource;
    spMaterialData m_Data;

    ezDynamicArray<RHI::spShaderSpecializationConstant> m_SpecializationConstants;
    ezDynamicArray<Property> m_Properties;
  };
}; // namespace RPI
