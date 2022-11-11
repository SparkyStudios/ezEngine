#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>

#include <Core/World/EventMessageHandlerComponent.h>

struct ezMsgGenericEvent;
struct ezMsgAnimationPoseUpdated;

typedef ezAudioSystemComponentManager<class ezAudioAnimationComponent> ezAudioAnimationComponentManager;

/// \brief A single entry in the audio animation component.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioAnimationEntry : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioAnimationEntry, ezReflectedClass);

public:
  ezAudioAnimationEntry();

  /// \brief Gets the name of the skeleton joint from which collect transformation data
  /// when activating the audio trigger.
  /// \param szName The name of the skeleton joint.
  void SetJointName(const char* szName);

  /// \returns The name of the skeleton joint from which collect transformation data.
  [[nodiscard]] const char* GetJointName() const;

  /// \brief The animation event to listen.
  ezString m_sEventName;

  /// \brief The audio trigger to activate when the animation
  /// event is triggered.
  ezString m_sTriggerName;

private:
  friend class ezAudioAnimationComponent;

  void ActivateTrigger() const;
  void Initialize(bool bSync);
  void UnloadTrigger();

  /// \brief (Optional) The name of the joint in the skeleton asset from which
  /// to copy position and orientation data. If not specified, the owner's transformation
  /// is used instead.
  ezHashedString m_sJointName;

  ezAudioSystemDataID m_uiEntityId;
  ezAudioSystemDataID m_uiEventId;
  ezUInt16 m_uiJointIndex;
  bool m_bTriggerLoaded;

  ezAudioSystemTransform m_LastTransform;
};

/// \brief Component that can be used to activate an audio trigger when animation events got triggered.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioAnimationComponent : public ezEventMessageHandlerComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioAnimationComponent, ezEventMessageHandlerComponent, ezAudioAnimationComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAudioAnimationComponent

protected:
  void Update();
  void OnAnimationPoseUpdated(ezMsgAnimationPoseUpdated& msg);
  void OnAnimationEvent(ezMsgGenericEvent& msg) const;

private:
  ezDynamicArray<ezAudioAnimationEntry> m_EventEntries;
};
