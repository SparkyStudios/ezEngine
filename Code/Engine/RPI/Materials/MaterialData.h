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
    EZ_DECLARE_POD_TYPE();

    ezColor m_AlbedoColor{ezColor::White};
    ezColor m_SpecularColor{ezColor::Black};
    ezColor m_EmissiveColor{ezColor::Black};

    spShaderVec2 m_UVTiling{1.0f, 1.0f};
    spShaderVec2 m_UVOffset{0.0f, 0.0f};

    float m_Roughness{0.0f};
    float m_Metalness{0.0f};
    float m_NormalIntensity{1.0f};
    float m_Height{1.0f};

    float m_WorldSpaceHeight{1.0f};
    float m_IOR{1.0f};
    float m_SubsurfaceScattering{0.0f};
    ezUInt32 m_Flags{0};

    spShaderVec3 m_SheenTint{1.0f, 1.0f, 1.0f};
    float m_Sheen{0.0f};

    float m_Anisotropic{0.0f};
    float m_AnisotropicRotation{0.0f};
    float m_Clearcoat{0.0f};
    float m_ClearcoatRoughness{0.0f};
  };
}

#include <RPI/Implementation/Materials/MaterialData.inl>
