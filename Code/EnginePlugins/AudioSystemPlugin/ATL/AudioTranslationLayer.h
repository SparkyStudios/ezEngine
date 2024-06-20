#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/ATL/AudioTranslationLayerData.h>
#include <AudioSystemPlugin/Core/AudioMiddleware.h>
#include <AudioSystemPlugin/Core/AudioThread.h>

struct ezCVarEvent;

/// \brief The Audio Translation Layer
///
/// This class is the bridge between the audio system and the audio middleware
/// and it is responsible of the execution of audio requests. It also stores the
/// registered audio controls so they can be retrieved during runtime by their names.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioTranslationLayer
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezAudioTranslationLayer);

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

  /// \brief Returns the ID of a trigger control identified by its name.
  /// \param sTriggerName The name of the trigger control.
  /// \returns The ID of the trigger, or 0 if a trigger control with the given name
  /// is not registered.
  [[nodiscard]] ezAudioSystemDataID GetTriggerId(ezStringView sTriggerName) const;

  /// \brief Returns the ID of a real-time parameter control identified by its name.
  /// \param sRtpcName The name of the RTPC.
  /// \returns The ID of the RTPC, or 0 if a RTPC with the given name is not registered.
  [[nodiscard]] ezAudioSystemDataID GetRtpcId(ezStringView sRtpcName) const;

  /// \brief Returns the ID of a switch state control identified by its name.
  /// \param sSwitchStateName The name of the switch state control.
  /// \returns The ID of the switch state, or 0 if a switch state control with the given name
  /// is not registered.
  [[nodiscard]] ezAudioSystemDataID GetSwitchStateId(ezStringView sSwitchStateName) const;

  /// \brief Returns the ID of an environment control identified by its name.
  /// \param sEnvironmentName The name of the environment control.
  /// \returns The ID of the environment, or 0 if an environment control with the given name
  /// is not registered.
  [[nodiscard]] ezAudioSystemDataID GetEnvironmentId(ezStringView sEnvironmentName) const;

private:
  friend class ezAudioSystem;

  bool ProcessRequest(ezVariant&& request, bool bSync);
  void OnMasterGainChange(const ezCVarEvent& e) const;
  void OnMuteChange(const ezCVarEvent& e) const;

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
  ezATLSoundBankLookup m_mSoundBanks;

  ezTime m_LastUpdateTime;
  ezTime m_LastFrameTime;

  ezAudioMiddleware* m_pAudioMiddleware{nullptr};
};
