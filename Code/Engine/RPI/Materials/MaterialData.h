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

#include <RPI/Shaders/ShaderTypes.h>

namespace RPI
{
  struct alignas(16) spMaterialData
  {
    ezColor m_Color;

    spShaderVec2 m_UVTiling;
    spShaderVec2 m_UVOffset;

    float m_Roughness;
    float m_Metalness;
    float m_NormalIntensity;
    float m_Height;

    float m_WorldSpaceHeight;
    float m_IOR;
    float m_SubsurfaceScattering;
    ezUInt32 m_Flags;

    spShaderVec3 m_SheenTint;
    float m_Sheen;

    float m_Anisotropic;
    float m_AnisotropicRotation;
    float m_Clearcoat;
    float m_ClearcoatRoughness;
  };
}
