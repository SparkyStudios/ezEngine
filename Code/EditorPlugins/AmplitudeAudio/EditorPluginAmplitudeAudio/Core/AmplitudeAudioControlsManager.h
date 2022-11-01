#pragma once

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <Foundation/Configuration/Singleton.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#define AUDIO_BIT(x) (1 << (x))

using namespace SparkyStudios::Audio;

class ezJSONReader;

enum AmplitudeControlType
{
  eAMCT_INVALID,
  eAMCT_AMPLITUDE_EVENT = AUDIO_BIT(0),
  eAMCT_AMPLITUDE_RTPC = AUDIO_BIT(1),
  eAMCT_AMPLITUDE_SOUND_BANK = AUDIO_BIT(2),
  eAMCT_AMPLITUDE_SWITCH = AUDIO_BIT(3),
  eAMCT_AMPLITUDE_SWITCH_STATE = AUDIO_BIT(4),
  eAMCT_AMPLITUDE_BUS = AUDIO_BIT(5),
  eAMCT_AMPLITUDE_EFFECT = AUDIO_BIT(6),
  eAMCT_AMPLITUDE_ENVIRONMENT = AUDIO_BIT(7),
};

class EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL ezAmplitudeAudioControlsManager
{
  EZ_DECLARE_SINGLETON(ezAmplitudeAudioControlsManager);

public:
  ezAmplitudeAudioControlsManager();
  ~ezAmplitudeAudioControlsManager() = default;

  /// \brief Create controls assets needed for the amplitude audio.
  /// \return EZ_SUCCESS on success, otherwise EZ_FAILURE.
  virtual ezResult ReloadControls();

private:
  virtual ezResult SerializeTriggerControl(ezStreamWriter* pStream, const ezAudioSystemTriggerData* pControlData);
  virtual ezResult SerializeRtpcControl(ezStreamWriter* pStream, const ezAudioSystemRtpcData* pControlData);
  virtual ezResult SerializeSwitchStateControl(ezStreamWriter* pStream, const ezAudioSystemSwitchStateData* pControlData);
  virtual ezResult SerializeEnvironmentControl(ezStreamWriter* pStream, const ezAudioSystemEnvironmentData* pControlData);

  virtual ezResult CreateTriggerControl(const char* szControlName, const ezAudioSystemTriggerData* pControlData);
  virtual ezResult CreateRtpcControl(const char* szControlName, const ezAudioSystemRtpcData* pControlData);
  virtual ezResult CreateSwitchStateControl(const char* szControlName, const ezAudioSystemSwitchStateData* pControlData);
  virtual ezResult CreateEnvironmentControl(const char* szControlName, const ezAudioSystemEnvironmentData* pControlData);

  void LoadSoundBanks(const char* sRootFolder, const char* sSubPath);
  //  void LoadBuses(const char* sRootFolder);
  ezResult LoadControlsInFolder(const char* sFolderPath, AmplitudeControlType type);
  ezResult LoadControl(const ezVariantDictionary& json, AmplitudeControlType type);
};
