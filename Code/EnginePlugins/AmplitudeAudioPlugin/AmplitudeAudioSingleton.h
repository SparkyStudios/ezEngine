#pragma once

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AudioSystemPlugin/Core/AudioMiddleware.h>

#include <Core/ResourceManager/ResourceHandle.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Types/UniquePtr.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

struct ezGameApplicationExecutionEvent;
class ezOpenDdlWriter;
class ezOpenDdlReaderElement;
typedef ezDynamicArray<ezUInt8> ezDataBuffer;

/// \brief The Amplitude configuration to be used on a specific platform
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeConfiguration
{
  ezString m_sInitSoundBank;
  ezString m_sEngineConfigFileName;

  void Save(ezOpenDdlWriter& ddl) const;
  void Load(const ezOpenDdlReaderElement& ddl);

  bool operator==(const ezAmplitudeConfiguration& rhs) const;
  bool operator!=(const ezAmplitudeConfiguration& rhs) const { return !operator==(rhs); }
};

/// \brief All available Amplitude platform configurations
struct EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeAssetProfiles
{
  ezResult Save(ezOpenDdlWriter& writer) const;
  ezResult Load(const ezOpenDdlReaderElement& reader);

  ezMap<ezString, ezAmplitudeConfiguration> m_AssetProfiles;
};

class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitude : public ezAudioMiddleware
{
private:
  EZ_DECLARE_SINGLETON_OF_INTERFACE(ezAmplitude, ezAudioMiddleware);

public:
  static void GameApplicationEventHandler(const ezGameApplicationExecutionEvent& e);

  ezAmplitude();
  ~ezAmplitude() override;

  [[nodiscard]] SparkyStudios::Audio::Amplitude::Engine* GetEngine() const { return m_pEngine; }

  ezResult SaveConfiguration(ezOpenDdlWriter& writer) override;
  ezResult LoadConfiguration(const ezOpenDdlReaderElement& reader) override;
  ezResult Startup() override;
  void Update(ezTime delta) override;
  ezResult Shutdown() override;
  ezResult Release() override;
  ezResult StopAllSounds() override;
  ezResult AddEntity(ezAudioSystemEntityData* pEntityData, const char* szEntityName) override;
  ezResult ResetEntity(ezAudioSystemEntityData* pEntityData) override;
  ezResult UpdateEntity(ezAudioSystemEntityData* pEntityData) override;
  ezResult RemoveEntity(ezAudioSystemEntityData* pEntityData) override;
  ezResult SetEntityTransform(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTransform& Transform) override;
  ezResult LoadTrigger(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTriggerData* pTriggerData, ezAudioSystemEventData* pEventData) override;
  ezResult ActivateTrigger(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTriggerData* pTriggerData, ezAudioSystemEventData* pEventData) override;
  ezResult UnloadTrigger(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTriggerData* pTriggerData) override;
  ezResult StopEvent(ezAudioSystemEntityData* pEntityData, const ezAudioSystemEventData* pEventData) override;
  ezResult StopAllEvents(ezAudioSystemEntityData* pEntityData) override;
  ezResult SetRtpc(ezAudioSystemEntityData* pEntityData, const ezAudioSystemRtpcData* pRtpcData, float fValue) override;
  ezResult ResetRtpc(ezAudioSystemEntityData* pEntityData, const ezAudioSystemRtpcData* pRtpcData) override;
  ezResult SetSwitchState(ezAudioSystemEntityData* pEntityData, const ezAudioSystemSwitchStateData* pSwitchStateData) override;
  ezResult SetObstructionAndOcclusion(ezAudioSystemEntityData* pEntityData, float fObstruction, float fOcclusion) override;
  ezResult SetEnvironmentAmount(ezAudioSystemEntityData* pEntityData, const ezAudioSystemEnvironmentData* pEnvironmentData, float fAmount) override;
  ezResult AddListener(ezAudioSystemListenerData* pListenerData, const char* szListenerName) override;
  ezResult ResetListener(ezAudioSystemListenerData* pListenerData) override;
  ezResult RemoveListener(ezAudioSystemListenerData* pListenerData) override;
  ezResult SetListenerTransform(ezAudioSystemListenerData* pListenerData, const ezAudioSystemTransform& Transform) override;
  ezResult LoadBank(ezAudioSystemBankData* pBankData) override;
  ezResult UnloadBank(ezAudioSystemBankData* pBankData) override;
  ezAudioSystemEntityData* CreateWorldEntity(ezAudioSystemDataID uiEntityId) override;
  ezAudioSystemEntityData* CreateEntityData(ezAudioSystemDataID uiEntityId) override;
  ezResult DestroyEntityData(ezAudioSystemEntityData* pEntityData) override;
  ezAudioSystemListenerData* CreateListenerData(ezAudioSystemDataID uiListenerId) override;
  ezResult DestroyListenerData(ezAudioSystemListenerData* pListenerData) override;
  ezAudioSystemEventData* CreateEventData(ezAudioSystemDataID uiEventId) override;
  ezResult ResetEventData(ezAudioSystemEventData* pEventData) override;
  ezResult DestroyEventData(ezAudioSystemEventData* pEventData) override;
  ezResult DestroyBank(ezAudioSystemBankData* pBankData) override;
  ezResult DestroyTriggerData(ezAudioSystemTriggerData* pTriggerData) override;
  ezResult DestroyRtpcData(ezAudioSystemRtpcData* pRtpcData) override;
  ezResult DestroySwitchStateData(ezAudioSystemSwitchStateData* pSwitchStateData) override;
  ezResult DestroyEnvironmentData(ezAudioSystemEnvironmentData* pEnvironmentData) override;
  ezResult SetLanguage(const char* szLanguage) override;
  [[nodiscard]] const char* GetMiddlewareName() const override;
  [[nodiscard]] const char* GetMiddlewareFolderName() const override;
  void OnMasterGainChange(float fGain) override;
  void OnMuteChange(bool bMute) override;
  void OnLoseFocus() override;
  void OnGainFocus() override;

  /// \brief Parses the implementation-specific entry that represent a bank.
  /// It's to the audi middleware to fill the struct with required data needed to locate or to store
  /// the file in memory. This data will be further used by LoadBank to load the bank.
  /// \param pBankEntry The stream storing the bank entry. Serialization/deserialization of that stream is implementation specific.
  /// \return The created bank data, or nullptr if no bank was created.
  ezAudioSystemBankData* DeserializeBankEntry(ezStreamReader* pBankEntry);

  /// \brief Parses the implementation-specific entry that represent a trigger.
  /// \param pTriggerEntry The stream storing the event entry. Serialization/deserialization of that stream is implementation specific.
  /// \return The created trigger data, or nullptr if no trigger was created.
  ezAudioSystemTriggerData* DeserializeTriggerEntry(ezStreamReader* pTriggerEntry) const;

  /// \brief Parses the implementation-specific entry that represent a rtpc.
  /// \param pRtpcEntry The stream storing the rtpc entry. Serialization/deserialization of that stream is implementation specific.
  /// \return The created rtpc data, or nullptr if no rtpc was created.
  ezAudioSystemRtpcData* DeserializeRtpcEntry(ezStreamReader* pRtpcEntry) const;

  /// \brief Parses the implementation-specific entry that represent a switch state.
  /// \param pSwitchStateEntry The stream storing the switch state entry. Serialization/deserialization of that stream is implementation specific.
  /// \return The created switch state data, or nullptr if no switch state was created.
  ezAudioSystemSwitchStateData* DeserializeSwitchStateEntry(ezStreamReader* pSwitchStateEntry) const;

  /// \brief Parses the implementation-specific entry that represent a environment effect.
  /// \param pEnvironmentEntry The stream storing the environment effect entry. Serialization/deserialization of that stream is implementation specific.
  /// \return The created environment effect data, or nullptr if no environment effect was created.
  ezAudioSystemEnvironmentData* DeserializeEnvironmentEntry(ezStreamReader* pEnvironmentEntry) const;

private:
  void DetectPlatform() const;

  SparkyStudios::Audio::Amplitude::Engine* m_pEngine;

  SparkyStudios::Audio::Amplitude::AmTime m_dCurrentTime;
  SparkyStudios::Audio::Amplitude::FileLoader m_Loader;

  bool m_bInitialized;

  struct Data
  {
    ezMap<ezString, float> m_VcaVolumes;
    ezAmplitudeAssetProfiles m_Configs;
    ezString m_sPlatform;
    SparkyStudios::Audio::Amplitude::AmObjectID m_uiInitSoundBank;
    ezHybridArray<ezDataBuffer*, 4> m_SbDeletionQueue;
  };

  ezUniquePtr<Data> m_pData;
};
