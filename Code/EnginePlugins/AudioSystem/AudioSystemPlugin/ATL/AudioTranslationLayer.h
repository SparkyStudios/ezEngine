#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/ATL/AudioTranslationLayerData.h>
#include <AudioSystemPlugin/Core/AudioMiddleware.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>
#include <AudioSystemPlugin/Core/AudioThread.h>

class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioTranslationLayer
{
public:
  ezAudioTranslationLayer();
  ~ezAudioTranslationLayer();

  /// \brief Initializes the audio translation layer.
  [[nodiscard]] ezResult Startup();

  /// \brief Shuts down the audio translation layer.
  void Shutdown();

  /// \brief Updates the audio translation layer.
  /// This will also trigger an update of the audio middleware.
  void Update();

  ezAudioSystemDataID GetTriggerId(const char* szTriggerName) const;

  ezAudioSystemDataID GetRtpcId(const char* szRtpcName) const;

  ezAudioSystemDataID GetSwitchStateId(const char* szSwitchStateName) const;

  ezAudioSystemDataID GetEnvironmentId(const char* szEnvironmentName) const;

private:
  friend class ezAudioSystem;

  void ProcessRequest(ezVariant&& request);

  void RegisterTrigger(ezAudioSystemDataID uiId, ezAudioSystemTriggerData* pTriggerData);
  void RegisterRtpc(ezAudioSystemDataID uiId, ezAudioSystemRtpcData* pRtpcData);
  void RegisterSwitchState(ezAudioSystemDataID uiId, ezAudioSystemSwitchStateData* pSwitchStateData);
  void RegisterEnvironment(ezAudioSystemDataID uiId, ezAudioSystemEnvironmentData* pEnvironmentData);

  void UnregisterEntity(ezAudioSystemDataID uiId);
  void UnregisterListener(ezAudioSystemDataID uiId);
  void UnregisterTrigger(ezAudioSystemDataID uiId);
  void UnregisterRtpc(ezAudioSystemDataID uiId);
  void UnregisterSwitchState(ezAudioSystemDataID uiId);
  void UnregisterEnvironment(ezAudioSystemDataID uiId);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  void DebugRender() const;
#endif

  // ATLObject containers
  ezATLEntityLookup m_mEntities;
  ezATLListenerLookup m_mListeners;
  ezATLTriggerLookup m_mTriggers;
  ezATLRtpcLookup m_mRtpcs;
  ezATLSwitchStateLookup m_mSwitchStates;
  ezATLEnvironmentLookup m_mEnvironments;
  // ezATLBanksLookup m_mBanks;

  ezTime m_LastUpdateTime;
  ezTime m_LastFrameTime;

  ezAudioMiddleware* m_pAudioMiddleware{nullptr};
};
