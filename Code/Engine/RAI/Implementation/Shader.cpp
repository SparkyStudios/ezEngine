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

#include <RAI/RAIPCH.h>

#include <RAI/Shader.h>

#include <RHI/Device.h>

namespace RAI
{
  void spShader::Clear()
  {
    if (!m_ShaderBytes.IsEmpty())
      EZ_DEFAULT_DELETE_ARRAY(m_ShaderBytes);
  }
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Shader);
