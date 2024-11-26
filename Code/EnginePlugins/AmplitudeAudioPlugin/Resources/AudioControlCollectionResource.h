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

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>

#include <Core/ResourceManager/Resource.h>

using ezAudioControlCollectionResourceHandle = ezTypedResourceHandle<class ezAmplitudeAudioControlCollectionResource>;

/// \brief Represents one audio control, used by a single audio middleware.
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioControlCollectionEntry
{
  ezString m_sName;                                               ///< Optional, can be used to look up the resource at runtime with a nice name.
  ezString m_sControlFile;                                        ///< The path to the audio system control.
  ezDefaultMemoryStreamStorage* m_pControlBufferStorage{nullptr}; ///< Buffer storage that contains the control data. Only have a value for loaded resources.
  ezEnum<ezAmplitudeAudioControlType> m_Type;                     ///< The type of the control.
};

/// \brief Describes a full ezAmplitudeAudioControlCollectionResource, i.e. lists all the controls that the collection contains.
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioControlCollectionResourceDescriptor
{
  ezDynamicArray<ezAmplitudeAudioControlCollectionEntry> m_Entries;

  void Save(ezStreamWriter& stream) const;
  void Load(ezStreamReader& stream);
};

/// \brief An ezAmplitudeAudioControlCollectionResource is used by the audio system to map a collection of audio controls to Amplitude objects.
///
/// For each audio control, the control name, the control type, and the path to the generated control file should be specified.
/// Those controls will now be able to be used by the audio system through components.
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioControlCollectionResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezAmplitudeAudioControlCollectionResource);
  EZ_RESOURCE_DECLARE_CREATEABLE(ezAudioControlCollectionResource, ezAmplitudeAudioControlCollectionResourceDescriptor);

public:
  ezAmplitudeAudioControlCollectionResource();

  /// \brief Registers this collection to the audio system.
  ///
  /// \note This is called automatically at initialization by the audio system on the control collection
  /// asset having the same name as the current audio middleware.
  ///
  /// Calling this twice has no effect.
  void Register();

  /// \brief Removes the registered controls from the audio system.
  ///
  /// Calling this twice has no effect.
  void Unregister();

  /// \brief Returns the resource descriptor for this resource.
  [[nodiscard]] const ezAmplitudeAudioControlCollectionResourceDescriptor& GetDescriptor() const;

private:
  static void RegisterTrigger(const char* szTriggerName, const char* szControlFile);
  static void RegisterTrigger(const char* szTriggerName, ezStreamReader* pStreamReader);
  static void UnregisterTrigger(const char* szTriggerName);

  static void RegisterRtpc(const char* szRtpcName, const char* szControlFile);
  static void RegisterRtpc(const char* szRtpcName, ezStreamReader* pStreamReader);
  static void UnregisterRtpc(const char* szRtpcName);

  static void RegisterSwitchState(const char* szSwitchStateName, const char* szControlFile);
  static void RegisterSwitchState(const char* szSwitchStateName, ezStreamReader* pStreamReader);
  static void UnregisterSwitchState(const char* szSwitchStateName);

  static void RegisterEnvironment(const char* szEnvironmentName, const char* szControlFile);
  static void RegisterEnvironment(const char* szEnvironmentName, ezStreamReader* pStreamReader);
  static void UnregisterEnvironment(const char* szEnvironmentName);

  static void RegisterSoundBank(const char* szBankName, const char* szControlFile);
  static void RegisterSoundBank(const char* szBankName, ezStreamReader* pStreamReader);
  static void UnregisterSoundBank(const char* szBankName);

  ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
  void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  bool m_bRegistered = false;
  ezAmplitudeAudioControlCollectionResourceDescriptor m_Collection;
};
