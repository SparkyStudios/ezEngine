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

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMaterialPropertyAttributeDefinition& attribute)
{
  inout_stream.WriteString(attribute.m_sName).AssertSuccess();
  inout_stream.WriteArray(attribute.m_Arguments).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMaterialPropertyAttributeDefinition& ref_attribute)
{
  ezString sName;
  inout_stream.ReadString(sName).AssertSuccess();
  ref_attribute.m_sName.Assign(sName);

  inout_stream.ReadArray(ref_attribute.m_Arguments).AssertSuccess();

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMaterialPropertyDefinition& property)
{
  inout_stream.WriteString(property.m_sName).AssertSuccess();
  inout_stream.WriteString(property.m_sType).AssertSuccess();

  inout_stream << property.m_Initializer;

  inout_stream.WriteArray(property.m_Attributes).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMaterialPropertyDefinition& ref_property)
{
  ezString sName;
  inout_stream.ReadString(sName).AssertSuccess();
  ref_property.m_sName.Assign(sName);

  inout_stream.ReadString(ref_property.m_sType).AssertSuccess();

  inout_stream >> ref_property.m_Initializer;

  inout_stream.ReadArray(ref_property.m_Attributes).AssertSuccess();

  return inout_stream;
}

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMaterialMetadata& metadata)
{
  inout_stream.WriteString(metadata.m_sName).AssertSuccess();
  inout_stream.WriteString(metadata.m_sDescription).AssertSuccess();

  inout_stream.WriteMap(metadata.m_Flags).AssertSuccess();
  inout_stream.WriteMap(metadata.m_Data).AssertSuccess();
  inout_stream.WriteMap(metadata.m_SpecializationConstants).AssertSuccess();
  inout_stream.WriteMap(metadata.m_Properties).AssertSuccess();

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMaterialMetadata& ref_metadata)
{
  ezString sName;
  inout_stream.ReadString(sName).AssertSuccess();
  ref_metadata.m_sName.Assign(sName);

  inout_stream.ReadString(ref_metadata.m_sDescription).AssertSuccess();

  inout_stream.ReadMap(ref_metadata.m_Flags).AssertSuccess();
  inout_stream.ReadMap(ref_metadata.m_Data).AssertSuccess();
  inout_stream.ReadMap(ref_metadata.m_SpecializationConstants).AssertSuccess();
  inout_stream.ReadMap(ref_metadata.m_Properties).AssertSuccess();

  return inout_stream;
}
