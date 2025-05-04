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

#include <RPI/RPIPCH.h>

#include <RPI/Assets/Material.h>

namespace RPI
{
  spMaterial::spMaterial()
    : m_hRootMaterialResource()
    , m_Data()
    , m_SpecializationConstants()
    , m_Properties()
  {
  }

  spMaterial::~spMaterial()
  {
    Clear();
  }

  void spMaterial::Clear()
  {
    m_Data = {};

    m_SpecializationConstants.Clear();
    m_Properties.Clear();
  }

  bool spMaterial::HasSpecializationConstant(const ezTempHashedString& sName) const
  {
    return std::any_of(begin(m_SpecializationConstants), end(m_SpecializationConstants), [&sName](const RHI::spShaderSpecializationConstant& constant)
      { return constant.m_sName == sName; });
  }

  void spMaterial::AddSpecializationConstant(const RHI::spShaderSpecializationConstant& constant)
  {
    if (HasSpecializationConstant(constant.m_sName))
      return;

    m_SpecializationConstants.PushBack(constant);
  }

  void spMaterial::RemoveSpecializationConstant(const ezTempHashedString& sName)
  {
    for (ezUInt32 i = 0, l = m_SpecializationConstants.GetCount(); i < l; ++i)
    {
      if (m_SpecializationConstants[i].m_sName == sName)
      {
        m_SpecializationConstants.RemoveAtAndCopy(i);
        return;
      }
    }
  }

  const RHI::spShaderSpecializationConstant* spMaterial::GetSpecializationConstant(const ezTempHashedString& sName) const
  {
    for (const auto& constant : m_SpecializationConstants)
      if (constant.m_sName == sName)
        return &constant;

    return nullptr;
  }

  void spMaterial::SetProperty(const ezStringView& sName, const ezVariant& value)
  {
    for (auto& property : m_Properties)
    {
      if (property.m_sName == sName)
      {
        property.m_Value = value;
        return;
      }
    }

    auto& prop = m_Properties.ExpandAndGetRef();
    prop.m_sName.Assign(sName);
    prop.m_Value = value;
  }

  ezVariant spMaterial::GetProperty(const ezTempHashedString& sName) const
  {
    for (const auto& property : m_Properties)
    {
      if (property.m_sName == sName)
        return property.m_Value;
    }

    return {};
  }

  const spMaterial::Property* spMaterial::GetProperty(ezUInt32 uiIndex) const
  {
    if (uiIndex < m_Properties.GetCount())
      return &m_Properties[uiIndex];

    return nullptr;
  }

  bool spMaterial::HasProperty(const ezTempHashedString& sName) const
  {
    return std::any_of(
      begin(m_Properties),
      end(m_Properties),
      [&sName](const Property& property)
      {
        return property.m_sName == sName;
      });
  }

  void spMaterial::SetRootMaterialResource(const spRootMaterialResourceHandle& hRootMaterialResource)
  {
    m_hRootMaterialResource = hRootMaterialResource;
  }

  ezUInt64 spMaterial::GetHeapMemoryUsage() const
  {
    return m_Properties.GetHeapMemoryUsage() + m_SpecializationConstants.GetHeapMemoryUsage();
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Assets_Material);
