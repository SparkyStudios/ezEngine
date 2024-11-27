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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <Core/Utils/CustomData.h>

/// \brief Custom data for the material of the room.
class ezAmplitudeRoomMaterial : public ezCustomData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeRoomMaterial, ezCustomData);

public:
  ezAmplitudeRoomMaterial();

  ezStaticArray<float, 9> m_absorptionCoefficients;
};

/// \brief Resource for the material of the room.
EZ_DECLARE_CUSTOM_DATA_RESOURCE(ezAmplitudeRoomMaterial);
