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

#include <RPI/Materials/MaterialParser.h>

namespace RPI
{
  class SP_RPI_DLL spRootMaterial
  {
    friend class spRootMaterialResource;
    friend class spRootMaterialResourceDescriptor;

#pragma region RPI Resource

  public:
    spRootMaterial();

    ~spRootMaterial();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spMaterialMetadata& GetMetadata() const { return m_Metadata; }

    [[nodiscard]] EZ_ALWAYS_INLINE ezByteArrayPtr GetShaderBytes() const { return m_ShaderBytes; }

  private:
    spMaterialMetadata m_Metadata;

    ezByteArrayPtr m_ShaderBytes;

#pragma endregion
  };
} // namespace RPI
