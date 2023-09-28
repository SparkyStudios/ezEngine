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

/// \brief Represents an invalid joint index.
#define spSkeletonInvalidJointIndex static_cast<ezUInt16>(0xFFFFu)

enum SP_RAI_CONSTANTS : ezUInt8
{
  /// \brief The maximum number of lod a mesh can have.
  SP_RAI_MAX_LOD_COUNT = 5,

  SP_RAI_MAX_MIP_COUNT = 12,
};

namespace RAI
{
  /// \brief A single vertex in a 3D mesh.
  struct spVertex
  {
    EZ_DECLARE_POD_TYPE();
    EZ_DECLARE_MEM_RELOCATABLE_TYPE();

    /// \brief The vertex position.
    ezVec3 m_vPosition;

    /// \brief The vertex normal.
    ezVec3 m_vNormal;

    /// \brief The vertex tangent.
    ezVec4 m_vTangent;

    /// \brief The vertex bitangent.
    ezVec4 m_vBiTangent;

    /// \brief The vertex texture coordinates at channel 1.
    ezVec2 m_vTexCoord0;

    /// \brief The vertex texture coordinates at channel 2.
    ezVec2 m_vTexCoord1;

    /// \brief The vertex color at channel 1.
    ezColor m_Color0;

    /// \brief The vertex color at channel 2.
    ezColor m_Color1;

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const spVertex& other) const
    {
      return m_vPosition == other.m_vPosition && m_vNormal == other.m_vNormal && m_vTangent == other.m_vTangent && m_vBiTangent == other.m_vBiTangent && m_vTexCoord0 == other.m_vTexCoord0 && m_vTexCoord1 == other.m_vTexCoord1 && m_Color0 == other.m_Color0 && m_Color1 == other.m_Color1;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const spVertex& other) const
    {
      return !(*this == other);
    }
  };

  /// \biref A permutation variable in a shader variant.
  struct SP_RAI_DLL spPermutationVar
  {
    EZ_DECLARE_MEM_RELOCATABLE_TYPE();

    /// \brief The name of the permutation variable.
    ezHashedString m_sName;

    /// \brief The value of the permutation variable.
    ezHashedString m_sValue;

    EZ_ALWAYS_INLINE bool operator==(const spPermutationVar& other) const { return m_sName == other.m_sName && m_sValue == other.m_sValue; }
  };

  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spVertex& vertex)
  {
    inout_stream << vertex.m_vPosition;
    inout_stream << vertex.m_vNormal;
    inout_stream << vertex.m_vTangent;
    inout_stream << vertex.m_vBiTangent;
    inout_stream << vertex.m_vTexCoord0;
    inout_stream << vertex.m_vTexCoord1;
    inout_stream << vertex.m_Color0;
    inout_stream << vertex.m_Color1;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spVertex& ref_vertex)
  {
    inout_stream >> ref_vertex.m_vPosition;
    inout_stream >> ref_vertex.m_vNormal;
    inout_stream >> ref_vertex.m_vTangent;
    inout_stream >> ref_vertex.m_vBiTangent;
    inout_stream >> ref_vertex.m_vTexCoord0;
    inout_stream >> ref_vertex.m_vTexCoord1;
    inout_stream >> ref_vertex.m_Color0;
    inout_stream >> ref_vertex.m_Color1;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spPermutationVar& permutation)
  {
    inout_stream << permutation.m_sName;
    inout_stream << permutation.m_sValue;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spPermutationVar& ref_permutation)
  {
    inout_stream >> ref_permutation.m_sName;
    inout_stream >> ref_permutation.m_sValue;

    return inout_stream;
  }
} // namespace RAI
