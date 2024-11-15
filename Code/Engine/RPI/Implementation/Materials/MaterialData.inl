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

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RPI::spMaterialData& material)
{
  inout_stream << material.m_AlbedoColor;
  inout_stream << material.m_SpecularColor;
  inout_stream << material.m_EmissiveColor;
  inout_stream << material.m_UVTiling;
  inout_stream << material.m_UVOffset;
  inout_stream << material.m_Roughness;
  inout_stream << material.m_Metalness;
  inout_stream << material.m_NormalIntensity;
  inout_stream << material.m_Height;
  inout_stream << material.m_WorldSpaceHeight;
  inout_stream << material.m_IOR;
  inout_stream << material.m_SubsurfaceScattering;
  inout_stream << material.m_Flags;
  inout_stream << material.m_SheenTint;
  inout_stream << material.m_Sheen;
  inout_stream << material.m_Anisotropic;
  inout_stream << material.m_AnisotropicRotation;
  inout_stream << material.m_Clearcoat;
  inout_stream << material.m_ClearcoatRoughness;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RPI::spMaterialData& ref_material)
{
  inout_stream >> ref_material.m_AlbedoColor;
  inout_stream >> ref_material.m_SpecularColor;
  inout_stream >> ref_material.m_EmissiveColor;
  inout_stream >> ref_material.m_UVTiling;
  inout_stream >> ref_material.m_UVOffset;
  inout_stream >> ref_material.m_Roughness;
  inout_stream >> ref_material.m_Metalness;
  inout_stream >> ref_material.m_NormalIntensity;
  inout_stream >> ref_material.m_Height;
  inout_stream >> ref_material.m_WorldSpaceHeight;
  inout_stream >> ref_material.m_IOR;
  inout_stream >> ref_material.m_SubsurfaceScattering;
  inout_stream >> ref_material.m_Flags;
  inout_stream >> ref_material.m_SheenTint;
  inout_stream >> ref_material.m_Sheen;
  inout_stream >> ref_material.m_Anisotropic;
  inout_stream >> ref_material.m_AnisotropicRotation;
  inout_stream >> ref_material.m_Clearcoat;
  inout_stream >> ref_material.m_ClearcoatRoughness;

  return inout_stream;
}
