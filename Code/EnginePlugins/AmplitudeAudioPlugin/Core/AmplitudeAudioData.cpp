// Copyright (c) 2022-present Sparky Studios. All rights reserved.
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

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioEntityData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioListenerData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioTriggerData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioRtpcData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioSwitchStateData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioEnvironmentData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioEventData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioSoundBankData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAmplitudeAudioControlType, 1)
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::Invalid),
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::Trigger),
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::Rtpc),
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::SoundBank),
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::Switch),
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::SwitchState),
  EZ_ENUM_CONSTANT(ezAmplitudeAudioControlType::Environment),
EZ_END_STATIC_REFLECTED_ENUM;
// clang-format on
