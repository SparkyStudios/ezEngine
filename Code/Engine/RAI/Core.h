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
