#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/ATL/AudioTranslationLayer.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>
#include <AudioSystemPlugin/Core/AudioThread.h>

#include <Core/GameApplication/GameApplicationBase.h>
#include <Core/Interfaces/SoundInterface.h>

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Threading/Semaphore.h>

typedef ezDeque<ezVariant> ezAudioSystemRequestsQueue;

/// \brief The AudioSystem.
///
/// This class is responsible to handle the communication between the
/// ATL and EZ. It implements the ezSoundInterface class and provides
/// methods to push audio requests in the requests queue (synchronously or asynchronously).
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystem final : public ezSoundInterface
{
  EZ_DECLARE_SINGLETON_OF_INTERFACE(ezAudioSystem, ezSoundInterface);

  // ----- ezSoundInterface

public:
  /// \brief This should be called in the audio middleware implementation startup to
  /// load the AudioSystem with the correct configuration.
  void LoadConfiguration(ezStringView sFile) override;

  /// \brief By default, the AudioSystem will auto-detect the platform (and thus the config) to use.
  /// Calling this before startup allows to override which configuration is used.
  void SetOverridePlatform(ezStringView sPlatform) override;

  /// \brief Called once per frame to update all sounds.
  void UpdateSound() override;

  /// \brief Asks the audio middleware to adjust its master volume.
  void SetMasterChannelVolume(float fVolume) override;

  /// \brief Gets the master volume of the audio middleware.
  float GetMasterChannelVolume() const override;

  /// \brief Asks the audio middleware to mute its master channel.
  void SetMasterChannelMute(bool bMute) override;

  /// \brief Gets the muted state of the audio middleware master channel.
  bool GetMasterChannelMute() const override;

  /// \brief Asks the audio middleware to pause every playbacks.
  void SetMasterChannelPaused(bool bPaused) override;

  /// \brief Gets the paused state of the audio middleware.
  bool GetMasterChannelPaused() const override;

  /// \brief Asks the audio middleware to adjust the volume of a sound group.
  void SetSoundGroupVolume(ezStringView sVcaGroupGuid, float fVolume) override;

  /// \brief Gets a sound group volume from the audio middleware.
  float GetSoundGroupVolume(ezStringView sVcaGroupGuid) const override;

  /// \brief Asks the audio middleware to set the required number of listeners.
  void SetNumListeners(ezUInt8 uiNumListeners) override {}

  /// \brief Gets the number of listeners from the audio middleware.
  ezUInt8 GetNumListeners() override;

  /// \brief Overrides the active audio middleware listener by the editor camera. Transformation
  /// data will be provided by the editor camera.
  void SetListenerOverrideMode(bool bEnabled) override;

  /// \brief Sets the transformation of the listener with the given ID.
  /// ID -1 is used for the override mode listener (editor camera).
  void SetListener(ezInt32 iIndex, const ezVec3& vPosition, const ezVec3& vForward, const ezVec3& vUp, const ezVec3& vVelocity) override;

  /// \brief Plays a sound once. Called by ezSoundInterface::PlaySound().
  ezResult OneShotSound(ezStringView sResourceID, const ezTransform& globalPosition, float fPitch = 1.0f, float fVolume = 1.0f, bool bBlockIfNotLoaded = true) override;

  // ----- ezAudioSystem

public:
  ezAudioSystem();
  virtual ~ezAudioSystem();

  bool Startup();
  void Shutdown();

  [[nodiscard]] bool IsInitialized() const;

  void SendRequest(ezVariant&& request);
  void SendRequests(ezAudioSystemRequestsQueue& requests);

  void SendRequestSync(ezVariant&& request);

  ezAudioSystemDataID GetTriggerId(ezStringView sTriggerName) const;

  ezAudioSystemDataID GetRtpcId(ezStringView sRtpcName) const;

  ezAudioSystemDataID GetSwitchStateId(ezStringView sSwitchStateName) const;

  ezAudioSystemDataID GetEnvironmentId(ezStringView sEnvironmentName) const;

  ezAudioSystemDataID GetBankId(ezStringView sBankName) const;

  void RegisterTrigger(ezAudioSystemDataID uiId, ezAudioSystemTriggerData* pTriggerData);
  void RegisterRtpc(ezAudioSystemDataID uiId, ezAudioSystemRtpcData* pRtpcData);
  void RegisterSwitchState(ezAudioSystemDataID uiId, ezAudioSystemSwitchStateData* pSwitchStateData);
  void RegisterEnvironment(ezAudioSystemDataID uiId, ezAudioSystemEnvironmentData* pEnvironmentData);
  void RegisterSoundBank(ezAudioSystemDataID uiId, ezAudioSystemBankData* pSoundBankData);

  void UnregisterEntity(ezAudioSystemDataID uiId);
  void UnregisterListener(ezAudioSystemDataID uiId);
  void UnregisterTrigger(ezAudioSystemDataID uiId);
  void UnregisterRtpc(ezAudioSystemDataID uiId);
  void UnregisterSwitchState(ezAudioSystemDataID uiId);
  void UnregisterEnvironment(ezAudioSystemDataID uiId);
  void UnregisterSoundBank(ezAudioSystemDataID uiId);

private:
  EZ_MAKE_SUBSYSTEM_STARTUP_FRIEND(AudioSystem, AudioSystemPlugin);

  friend class ezAudioThread;
  friend class ezAudioTranslationLayer;

  static void GameApplicationEventHandler(const ezGameApplicationExecutionEvent& e);

  void UpdateInternal();

  void StartAudioThread();
  void StopAudioThread();

  void QueueRequestCallback(ezVariant&& request, bool bSync);

  ezAudioThread* m_pAudioThread = nullptr;
  ezAudioTranslationLayer m_AudioTranslationLayer;

  ezAudioSystemRequestsQueue m_RequestsQueue;
  ezAudioSystemRequestsQueue m_PendingRequestsQueue;
  ezAudioSystemRequestsQueue m_BlockingRequestsQueue;
  ezAudioSystemRequestsQueue m_PendingRequestCallbacksQueue;
  ezAudioSystemRequestsQueue m_BlockingRequestCallbacksQueue;

  mutable ezMutex m_RequestsMutex;
  mutable ezMutex m_PendingRequestsMutex;
  mutable ezMutex m_BlockingRequestsMutex;
  mutable ezMutex m_PendingRequestCallbacksMutex;
  mutable ezMutex m_BlockingRequestCallbacksMutex;

  ezSemaphore m_MainEvent;
  ezSemaphore m_ProcessingEvent;

  bool m_bInitialized;

  bool m_bListenerOverrideMode;
};
