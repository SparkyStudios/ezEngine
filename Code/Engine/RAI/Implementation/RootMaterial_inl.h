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

namespace RAI
{
  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spRootMaterial::Permutation parameter)
  {
    inout_stream << parameter.m_sName;
    inout_stream << parameter.m_eType;
    inout_stream.WriteArray(parameter.m_AllowedValues).AssertSuccess();

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spRootMaterial::Permutation& parameter)
  {
    inout_stream >> parameter.m_sName;
    inout_stream >> parameter.m_eType;
    inout_stream.ReadArray(parameter.m_AllowedValues).AssertSuccess();

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spRootMaterial::ParameterDataType dataType)
  {
    inout_stream << dataType.m_eType;
    inout_stream << dataType.m_uiRowCount;
    inout_stream << dataType.m_uiColumnCount;
    inout_stream << dataType.m_uiArraySize;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spRootMaterial::ParameterDataType& dataType)
  {
    inout_stream >> dataType.m_eType;
    inout_stream >> dataType.m_uiRowCount;
    inout_stream >> dataType.m_uiColumnCount;
    inout_stream >> dataType.m_uiArraySize;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spRootMaterial::Parameter parameter)
  {
    inout_stream << parameter.m_sName;
    inout_stream << parameter.m_bIsPermutation;
    inout_stream << parameter.m_DataType;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spRootMaterial::Parameter& parameter)
  {
    inout_stream >> parameter.m_sName;
    inout_stream >> parameter.m_bIsPermutation;
    inout_stream >> parameter.m_DataType;

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const spRootMaterial::ParameterGroup& group)
  {
    inout_stream << group.m_sName;
    inout_stream.WriteArray(group.m_Parameters).AssertSuccess();

    return inout_stream;
  }

  EZ_FORCE_INLINE ezStreamReader& operator>>(ezStreamReader& inout_stream, spRootMaterial::ParameterGroup& group)
  {
    inout_stream >> group.m_sName;
    inout_stream.ReadArray(group.m_Parameters).AssertSuccess();

    return inout_stream;
  }
} // namespace RAI
