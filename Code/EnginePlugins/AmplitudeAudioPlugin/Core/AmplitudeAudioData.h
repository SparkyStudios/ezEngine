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

#pragma once

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common/Types.h>

/// \brief The type of control. This is used by control assets to determine the type of the control
/// when the audio system is parsing them.
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioControlType
{
  using StorageType = ezUInt8;

  enum Enum : StorageType
  {
    /// \brief The control is not known to the audio system.
    Invalid = 0,

    /// \brief The control is a source.
    Trigger = 1,

    /// \brief The control is a real-time parameter.
    Rtpc = 2,

    /// \brief The control is a sound bank.
    SoundBank = 3,

    /// \brief The control is a switch container.
    Switch = 4,

    /// \brief The control is a switch state.
    SwitchState = 5,

    /// \brief The control is an environment effect.
    Environment = 6,

    Default = Invalid,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_AMPLITUDEAUDIOPLUGIN_DLL, ezAmplitudeAudioControlType);

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioEntityData final : public ezAudioSystemEntityData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioEntityData, ezAudioSystemEntityData);

public:
  explicit ezAmplitudeAudioEntityData(const SparkyStudios::Audio::Amplitude::AmEntityID uiEntityID, bool bHasPosition = true)
    : ezAudioSystemEntityData()
    , m_bHasPosition(bHasPosition)
    , m_uiAmId(uiEntityID)
  {
  }

  bool m_bHasPosition;
  const SparkyStudios::Audio::Amplitude::AmEntityID m_uiAmId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioListenerData final : public ezAudioSystemListenerData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioListenerData, ezAudioSystemListenerData);

public:
  explicit ezAmplitudeAudioListenerData(const SparkyStudios::Audio::Amplitude::AmEntityID uiListenerID)
    : ezAudioSystemListenerData()
    , m_uiAmId(uiListenerID)
  {
  }

  const SparkyStudios::Audio::Amplitude::AmListenerID m_uiAmId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioTriggerData final : public ezAudioSystemTriggerData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioTriggerData, ezAudioSystemTriggerData);

public:
  explicit ezAmplitudeAudioTriggerData(const SparkyStudios::Audio::Amplitude::AmEventID uiEventId)
    : ezAudioSystemTriggerData()
    , m_uiAmId(uiEventId)
  {
  }

  const SparkyStudios::Audio::Amplitude::AmEventID m_uiAmId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioRtpcData final : public ezAudioSystemRtpcData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioRtpcData, ezAudioSystemRtpcData);

public:
  explicit ezAmplitudeAudioRtpcData(const SparkyStudios::Audio::Amplitude::AmRtpcID uiRtpcId)
    : ezAudioSystemRtpcData()
    , m_uiAmId(uiRtpcId)
  {
  }

  const SparkyStudios::Audio::Amplitude::AmRtpcID m_uiAmId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioSwitchStateData final : public ezAudioSystemSwitchStateData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioSwitchStateData, ezAudioSystemSwitchStateData);

public:
  explicit ezAmplitudeAudioSwitchStateData(const SparkyStudios::Audio::Amplitude::AmSwitchID uiSwitchId, const SparkyStudios::Audio::Amplitude::AmObjectID uiSwitchStateId)
    : ezAudioSystemSwitchStateData()
    , m_uiSwitchId(uiSwitchId)
    , m_uiSwitchStateId(uiSwitchStateId)
  {
  }

  const SparkyStudios::Audio::Amplitude::AmSwitchID m_uiSwitchId;
  const SparkyStudios::Audio::Amplitude::AmObjectID m_uiSwitchStateId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioEnvironmentData final : public ezAudioSystemEnvironmentData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioEnvironmentData, ezAudioSystemEnvironmentData);

public:
  explicit ezAmplitudeAudioEnvironmentData(const SparkyStudios::Audio::Amplitude::AmEnvironmentID uiEnvironmentId, const SparkyStudios::Audio::Amplitude::AmEffectID uiEffectId)
    : ezAudioSystemEnvironmentData()
    , m_uiAmId(uiEnvironmentId)
    , m_uiEffectId(uiEffectId)
  {
  }

  const SparkyStudios::Audio::Amplitude::AmEnvironmentID m_uiAmId;
  const SparkyStudios::Audio::Amplitude::AmEffectID m_uiEffectId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioEventData final : public ezAudioSystemEventData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioEventData, ezAudioSystemEventData);

public:
  explicit ezAmplitudeAudioEventData(const SparkyStudios::Audio::Amplitude::AmEventID uiEventId)
    : ezAudioSystemEventData()
    , m_uiAmId(uiEventId)
  {
  }

  ezAudioSystemEventState m_eState{ezAudioSystemEventState::Invalid};
  SparkyStudios::Audio::Amplitude::EventCanceler m_EventCanceler;
  const SparkyStudios::Audio::Amplitude::AmEventID m_uiAmId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioSoundBankData final : public ezAudioSystemBankData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioSoundBankData, ezAudioSystemBankData);

public:
  explicit ezAmplitudeAudioSoundBankData(const SparkyStudios::Audio::Amplitude::AmBankID uiBankId, const ezString& sFileName)
    : ezAudioSystemBankData()
    , m_uiAmId(uiBankId)
    , m_sFileName(sFileName)
  {
  }

  const SparkyStudios::Audio::Amplitude::AmBankID m_uiAmId;
  const ezString m_sFileName;
};
