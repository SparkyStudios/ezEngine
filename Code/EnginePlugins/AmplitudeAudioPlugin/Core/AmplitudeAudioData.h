#pragma once

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioEntityData : public ezAudioSystemEntityData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioEntityData, ezAudioSystemEntityData);

public:
  explicit ezAmplitudeAudioEntityData(const SparkyStudios::Audio::Amplitude::AmEntityID uiEntityID, bool bHasPosition = true)
    : ezAudioSystemEntityData()
    , m_uiAmId(uiEntityID)
    , m_bHasPosition(bHasPosition)
  {
  }

  bool m_bHasPosition;
  const SparkyStudios::Audio::Amplitude::AmEntityID m_uiAmId;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioListenerData : public ezAudioSystemListenerData
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

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioTriggerData : public ezAudioSystemTriggerData
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

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioRtpcData : public ezAudioSystemRtpcData
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

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioSwitchStateData : public ezAudioSystemSwitchStateData
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

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioEnvironmentData : public ezAudioSystemEnvironmentData
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

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioEventData : public ezAudioSystemEventData
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
