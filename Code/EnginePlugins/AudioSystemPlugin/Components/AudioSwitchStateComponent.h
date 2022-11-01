#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>
#include <AudioSystemPlugin/Core/AudioSystemMessages.h>

typedef ezComponentManager<class ezAudioSwitchStateComponent, ezBlockStorageType::FreeList> ezAudioSwitchStateComponentManager;

/// \brief Component used to set the current state of a switch in the audio middleware.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSwitchStateComponent : public ezAudioSystemProxyDependentComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioSwitchStateComponent, ezAudioSystemProxyDependentComponent, ezAudioSwitchStateComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAudioSystemComponent

private:
  void ezAudioSystemComponentIsAbstract() override {}

  // ezAudioSwitchStateComponent

public:
  ezAudioSwitchStateComponent();
  ~ezAudioSwitchStateComponent() override;

  /// \brief Sets the active state of the switch. This will send a request to the Audio System.
  ///
  /// \param sSwitchStateName The name of the new state the switch should have.
  /// \param bSync Whether the request should be sent synchronously or asynchronously.
  void SetState(const ezString& sSwitchStateName, bool bSync = false);

  /// \brief Gets the current state of the switch.
  /// \returns The current state of the switch.
  EZ_NODISCARD const ezString& GetState() const;

  /// \brief Event that is triggered when the component receives a
  /// SetSwitchState message.
  void OnSetState(ezMsgAudioSystemSetSwitchState& msg);

private:
  ezString m_sCurrentSwitchState;
  ezString m_sInitialSwitchState;

  ezEventMessageSender<ezMsgAudioSystemSwitchStateChanged> m_ValueChangedEventSender;
};
