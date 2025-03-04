#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Core/AudioSystemMessages.h>

#include <Foundation/Types/VariantTypeRegistry.h>

// clang-format off
EZ_IMPLEMENT_MESSAGE_TYPE(ezMsgAudioSystemSetRtpcValue);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMsgAudioSystemSetRtpcValue, 1, ezRTTIDefaultAllocator<ezMsgAudioSystemSetRtpcValue>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_fValue)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
    EZ_MEMBER_PROPERTY("Sync", m_bSync)->AddAttributes(new ezDefaultValueAttribute(false)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_IMPLEMENT_MESSAGE_TYPE(ezMsgAudioSystemRtpcValueChanged);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMsgAudioSystemRtpcValueChanged, 1, ezRTTIDefaultAllocator<ezMsgAudioSystemRtpcValueChanged>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_fValue)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_IMPLEMENT_MESSAGE_TYPE(ezMsgAudioSystemSetSwitchState);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMsgAudioSystemSetSwitchState, 1, ezRTTIDefaultAllocator<ezMsgAudioSystemSetSwitchState>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("State", m_sSwitchState),
    EZ_MEMBER_PROPERTY("Sync", m_bSync)->AddAttributes(new ezDefaultValueAttribute(false)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_IMPLEMENT_MESSAGE_TYPE(ezMsgAudioSystemSwitchStateChanged);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMsgAudioSystemSwitchStateChanged, 1, ezRTTIDefaultAllocator<ezMsgAudioSystemSwitchStateChanged>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("State", m_sSwitchState),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_IMPLEMENT_MESSAGE_TYPE(ezMsgAudioSystemSetEnvironmentAmount);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMsgAudioSystemSetEnvironmentAmount, 1, ezRTTIDefaultAllocator<ezMsgAudioSystemSetEnvironmentAmount>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Value", m_fAmount)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
    EZ_MEMBER_PROPERTY("Sync", m_bSync)->AddAttributes(new ezDefaultValueAttribute(false)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_AudioSystemMessages);
