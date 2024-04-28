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

#include <RAI/Core.h>

namespace RAI
{
  /// \brief A root material asset.
  ///
  /// Reflects an SPSL binary root material.
  class SP_RAI_DLL spRootMaterial
  {
    friend class spRootMaterialResource;
    friend class spRootMaterialResourceDescriptor;

  public:
    struct PermutationType
    {
      typedef ezUInt8 StorageType;

      enum Enum : StorageType
      {
        Boolean,
        Enumeration,
        Integer,

        Default = Boolean
      };
    };

    struct ParameterType
    {
      typedef ezUInt8 StorageType;

      enum Enum : StorageType
      {
        Boolean,
        Integer,
        UnsignedInteger,
        Float,
        Double,
        Texture,
        Sampler,

        Default = Boolean
      };
    };

    struct Permutation
    {
      ezHashedString m_sName;
      ezEnum<PermutationType> m_eType;
      ezDynamicArray<ezString> m_AllowedValues;
    };

    struct ParameterDataType
    {
      ezEnum<ParameterType> m_eType;
      ezUInt32 m_uiRowCount;
      ezUInt32 m_uiColumnCount;
      ezUInt32 m_uiArraySize;
    };

    struct Parameter
    {
      ezHashedString m_sName;
      bool m_bIsPermutation;
      ParameterDataType m_DataType;
    };

    struct ParameterGroup
    {
      ezHashedString m_sName;
      ezDynamicArray<Parameter> m_Parameters;

      EZ_FORCE_INLINE bool operator==(const ParameterGroup& other) const
      {
        return m_sName == other.m_sName && ezHashingUtils::xxHash32(m_Parameters.GetData(), m_Parameters.GetCount() * sizeof(Parameter)) == ezHashingUtils::xxHash32(other.m_Parameters.GetData(), other.m_Parameters.GetCount() * sizeof(Parameter));
      }
    };

    spRootMaterial() = default;

    void Clear();

    ezHashedString m_sName;

    ezDynamicArray<ParameterGroup> m_ParameterGroups;

    ezDynamicArray<Permutation> m_Permutations;
  };
} // namespace RAI

#include <RAI/Implementation/RootMaterial_inl.h>
