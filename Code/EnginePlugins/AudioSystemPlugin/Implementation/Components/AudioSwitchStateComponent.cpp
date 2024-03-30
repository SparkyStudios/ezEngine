#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioSwitchStateComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

constexpr ezTypeVersion kVersion_AudioSwitchStateComponent = 1;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioSwitchStateComponent, kVersion_AudioSwitchStateComponent, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("InitialValue", m_sInitialSwitchState),

    EZ_ACCESSOR_PROPERTY_READ_ONLY("State", GetState)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(SetState, In, "State", In, "Sync"),
    EZ_SCRIPT_FUNCTION_PROPERTY(GetState),
  }
  EZ_END_FUNCTIONS;

  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgAudioSystemSetSwitchState, OnSetState),
  }
  EZ_END_MESSAGEHANDLERS;

  EZ_BEGIN_MESSAGESENDERS
  {
    EZ_MESSAGE_SENDER(m_ValueChangedEventSender),
  }
  EZ_END_MESSAGESENDERS;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezAudioSwitchStateComponent::Initialize()
{
  SUPER::Initialize();

  SetState(m_sInitialSwitchState, false);
}

void ezAudioSwitchStateComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioSwitchStateComponent);

  s << m_sInitialSwitchState;
}

void ezAudioSwitchStateComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioSwitchStateComponent);

  s >> m_sInitialSwitchState;
}

ezAudioSwitchStateComponent::ezAudioSwitchStateComponent()
  : ezAudioSystemProxyDependentComponent()
{
}

ezAudioSwitchStateComponent::~ezAudioSwitchStateComponent() = default;

void ezAudioSwitchStateComponent::SetState(const ezString& sSwitchStateName, bool bSync)
{
  if (m_sCurrentSwitchState == sSwitchStateName)
    return; // No need to update...

  ezAudioSystemRequestSetSwitchState request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetSwitchStateId(sSwitchStateName);

  request.m_Callback = [this, sSwitchStateName](const ezAudioSystemRequestSetSwitchState& e)
  {
    if (e.m_eStatus.Failed())
      return;

    // Save the value in the component
    m_sCurrentSwitchState = sSwitchStateName;

    // Notify for the change
    ezMsgAudioSystemSwitchStateChanged msg;
    msg.m_sSwitchState = sSwitchStateName;

    // We are not in the writing thread, so posting the message for the next frame instead of sending it now...
    m_ValueChangedEventSender.PostEventMessage(msg, this, GetOwner(), ezTime::MakeZero(), ezObjectMsgQueueType::NextFrame);
  };

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(request);
  }
}

const ezString& ezAudioSwitchStateComponent::GetState() const
{
  return m_sCurrentSwitchState;
}

void ezAudioSwitchStateComponent::OnSetState(ezMsgAudioSystemSetSwitchState& msg)
{
  SetState(msg.m_sSwitchState, msg.m_bSync);
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioSwitchStateComponent);
