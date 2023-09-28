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

#include <RAI/RAIDLL.h>

#include <Foundation/IO/MemoryStream.h>

#include <RHI/Input.h>
#include <RHI/Shader.h>

#include <RAI/Resources/ShaderVariantResource.h>

namespace RAI
{
  /// \brief A shader asset.
  ///
  /// Reflects an SPSL binary material.
  class SP_RAI_DLL spShader
  {
    friend class spShaderResource;
    friend class spShaderResourceDescriptor;
    friend class spShaderResourceLoader;

  public:
    spShader() = default;

  private:
    ezStringView m_sName;

    ezDynamicArray<spShaderVariantResourceHandle> m_Variants;
  };
} // namespace RAI
