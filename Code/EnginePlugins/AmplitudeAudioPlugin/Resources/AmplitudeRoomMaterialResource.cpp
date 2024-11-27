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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/Resources/AmplitudeRoomMaterialResource.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeRoomMaterial, 1, ezRTTIDefaultAllocator<ezAmplitudeRoomMaterial>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("AbsorptionCoefficients", m_absorptionCoefficients)->AddAttributes(new ezMaxArraySizeAttribute(9)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

EZ_DEFINE_CUSTOM_DATA_RESOURCE(ezAmplitudeRoomMaterial);

ezAmplitudeRoomMaterial::ezAmplitudeRoomMaterial()
{
  m_absorptionCoefficients.SetCount(9, 1.00f);
}
