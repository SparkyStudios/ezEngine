#pragma once

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>

#include <Core/ResourceManager/Resource.h>

using ezAudioControlCollectionResourceHandle = ezTypedResourceHandle<class ezAmplitudeAudioControlCollectionResource>;

/// \brief Represents one audio control, used by a single audio middleware.
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioControlCollectionEntry
{
  ezString m_sName;                                               ///< Optional, can be used to lookup the resource at runtime with a nice name. E.g. "SkyTexture" instead of some GUID.
  ezString m_sControlFile;                                        ///< The path to the audio system control.
  ezDefaultMemoryStreamStorage* m_pControlBufferStorage{nullptr}; ///< Buffer storage that contains the control data. Only have a value for loaded resources.
  ezEnum<ezAmplitudeAudioControlType> m_Type;                     ///< The type of the control.
};

/// \brief Describes a full ezAmplitudeAudioControlCollectionResource, ie. lists all the controls that the collection contains.
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAudioControlCollectionResourceDescriptor
{
  ezDynamicArray<ezAmplitudeAudioControlCollectionEntry> m_Entries;

  void Save(ezStreamWriter& stream) const;
  void Load(ezStreamReader& stream);
};

/// \brief An ezAmplitudeAudioControlCollectionResource is used by the audio system to map a collection of audio controls to a single audio middleware.
///
/// For each audio control should specify the control name, the control type, and the path to the generated middleware control file.
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
  /// asset having the same name than the current audio middleware.
  ///
  /// Calling this twice has no effect.
  void Register();

  /// \brief Removes the registered controls from the audio system.
  ///
  /// Calling this twice has no effect.
  void Unregister();

  /// \brief Returns the resource descriptor for this resource.
  EZ_NODISCARD const ezAmplitudeAudioControlCollectionResourceDescriptor& GetDescriptor() const;

private:
  void RegisterTrigger(const char* szTriggerName, const char* szControlFile);
  void RegisterTrigger(const char* szTriggerName, ezStreamReader* pStreamReader);
  void UnregisterTrigger(const char* szTriggerName);

  void RegisterRtpc(const char* szRtpcName, const char* szControlFile);
  void RegisterRtpc(const char* szRtpcName, ezStreamReader* pStreamReader);
  void UnregisterRtpc(const char* szRtpcName);

  void RegisterSwitchState(const char* szSwitchStateName, const char* szControlFile);
  void RegisterSwitchState(const char* szSwitchStateName, ezStreamReader* pStreamReader);
  void UnregisterSwitchState(const char* szSwitchStateName);

  void RegisterEnvironment(const char* szEnvironmentName, const char* szControlFile);
  void RegisterEnvironment(const char* szEnvironmentName, ezStreamReader* pStreamReader);
  void UnregisterEnvironment(const char* szEnvironmentName);

  void RegisterSoundBank(const char* szBankName, const char* szControlFile);
  void RegisterSoundBank(const char* szBankName, ezStreamReader* pStreamReader);
  void UnregisterSoundBank(const char* szBankName);

  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  bool m_bRegistered = false;
  ezAmplitudeAudioControlCollectionResourceDescriptor m_Collection;
};
