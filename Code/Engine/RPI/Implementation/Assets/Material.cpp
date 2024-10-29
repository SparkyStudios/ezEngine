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
  {}

  spMaterial::~spMaterial()
  {
    Clear();
  }

  const RHI::spShaderSpecializationConstant* spMaterial::GetSpecializationConstant(const ezTempHashedString& name)
  {
    for (const auto& constant : m_SpecializationConstants)
      if (constant.m_sName == name)
        return &constant;

    return nullptr;
  }

  void spMaterial::SetProperty(const ezTempHashedString& name, const ezVariant& value)
  {
    for (auto& property : m_Properties)
    {
      if (property.m_sName == name)
      {
        property.m_Value = value;
        return;
      }
    }
  }

  ezVariant spMaterial::GetProperty(const ezTempHashedString& name)
  {
    for (const auto& property : m_Properties)
    {
      if (property.m_sName == name)
        return property.m_Value;
    }

    return ezVariant();
  }

}
