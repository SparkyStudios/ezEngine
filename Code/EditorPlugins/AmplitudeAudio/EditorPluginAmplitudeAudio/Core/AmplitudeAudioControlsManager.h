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

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <Foundation/Configuration/Singleton.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio;

class ezJSONReader;

class EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL ezAmplitudeAudioControlsManager
{
  EZ_DECLARE_SINGLETON(ezAmplitudeAudioControlsManager);

public:
  ezAmplitudeAudioControlsManager();
  ~ezAmplitudeAudioControlsManager() = default;

  /// \brief Create controls assets needed for Amplitude Audio.
  /// \return EZ_SUCCESS on success, otherwise EZ_FAILURE.
  ezResult ReloadControls();

private:
  ezResult SerializeTriggerControl(ezStreamWriter* pStream, const ezAudioSystemTriggerData* pControlData);
  ezResult SerializeRtpcControl(ezStreamWriter* pStream, const ezAudioSystemRtpcData* pControlData);
  ezResult SerializeSwitchStateControl(ezStreamWriter* pStream, const ezAudioSystemSwitchStateData* pControlData);
  ezResult SerializeEnvironmentControl(ezStreamWriter* pStream, const ezAudioSystemEnvironmentData* pControlData);
  ezResult SerializeSoundBankControl(ezStreamWriter* pStream, const ezAudioSystemBankData* pControlData);

  ezResult CreateTriggerControl(const char* szControlName, const ezAudioSystemTriggerData* pControlData);
  ezResult CreateRtpcControl(const char* szControlName, const ezAudioSystemRtpcData* pControlData);
  ezResult CreateSwitchStateControl(const char* szControlName, const ezAudioSystemSwitchStateData* pControlData);
  ezResult CreateEnvironmentControl(const char* szControlName, const ezAudioSystemEnvironmentData* pControlData);
  ezResult CreateSoundBankControl(const char* szControlName, const ezAudioSystemBankData* pControlData);

  ezResult LoadSoundBanks(const char* szRootFolder, const char* szSubPath);
  //  void LoadBuses(const char* sRootFolder);
  ezResult LoadControlsInFolder(const char* szFolderPath, const ezEnum<ezAmplitudeAudioControlType>& eType);
  ezResult LoadControl(const ezVariantDictionary& json, const ezEnum<ezAmplitudeAudioControlType>& eType);
};
