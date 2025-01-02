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

namespace RPI
{
  struct spMaterialPropertyAttributeDefinition
  {
    ezHashedString m_sName;
    ezHybridArray<ezVariant, 8> m_Arguments;
  };

  struct spMaterialPropertyDefinition
  {
    const ezRTTI* m_pType = nullptr;
    ezString m_sType;

    ezHashedString m_sName;
    ezVariant m_Initializer;

    ezHybridArray<spMaterialPropertyAttributeDefinition, 4> m_Attributes;
  };

  struct spMaterialMetadata
  {
    ezHashedString m_sName;
    ezString m_sDescription;

    ezMap<ezHashedString, ezString> m_Macros;
    ezMap<ezUInt32, ezVariant> m_Flags;
    ezMap<ezHashedString, ezVariant> m_Data;
    ezMap<ezHashedString, ezVariant> m_SpecializationConstants;
    ezMap<ezHashedString, spMaterialPropertyDefinition> m_Properties;
  };

  class SP_RPI_DLL spMaterialParser
  {
  public:
    static ezResult ParseMaterialMetadata(ezStringView sMaterialFilePath, spMaterialMetadata& out_MaterialMetadata);
  };
} // namespace RPI

#include <RPI/Implementation/Materials/MaterialParser_inl.h>
