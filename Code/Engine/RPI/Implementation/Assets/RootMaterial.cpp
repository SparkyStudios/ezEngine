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

#include <RPI/Assets/RootMaterial.h>

#include <RHI/Device.h>

namespace RPI
{
  spRootMaterial::spRootMaterial()
    : m_Metadata()
    , m_ShaderBytes()
  {
  }

  spRootMaterial::spRootMaterial(const spRootMaterial& other)
  {
    *this = other;
  }

  spRootMaterial::spRootMaterial(spRootMaterial&& other) noexcept
  {
    *this = std::move(other);
  }

  spRootMaterial::~spRootMaterial()
  {
    Clear();
  }

  void spRootMaterial::Clear()
  {
    m_Metadata.m_sName.Clear();
    m_Metadata.m_sDescription.Clear();
    m_Metadata.m_Flags.Clear();
    m_Metadata.m_Data.Clear();
    m_Metadata.m_SpecializationConstants.Clear();
    m_Metadata.m_Properties.Clear();

    if (m_ShaderBytes.GetPtr() != nullptr)
      EZ_DEFAULT_DELETE_ARRAY(m_ShaderBytes);
  }

  spRootMaterial& spRootMaterial::operator=(const spRootMaterial& other)
  {
    m_Metadata = other.m_Metadata;

    m_ShaderBytes = EZ_DEFAULT_NEW_ARRAY(ezUInt8, other.m_ShaderBytes.GetCount());
    m_ShaderBytes.CopyFrom(other.m_ShaderBytes);

    return *this;
  }

  spRootMaterial& spRootMaterial::operator=(spRootMaterial&& other) noexcept
  {
    Clear();

    ezMath::Swap(m_Metadata, other.m_Metadata);
    ezMath::Swap(m_ShaderBytes, other.m_ShaderBytes);

    other.m_ShaderBytes.Clear();
    other.Clear();

    return *this;
  }
} // namespace RPI
