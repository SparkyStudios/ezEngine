#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(ezAudioSystemTransform, ezNoBase, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Position", m_vPosition),
    EZ_MEMBER_PROPERTY("Velocity", m_vVelocity),
    EZ_MEMBER_PROPERTY("Forward", m_vForward),
    EZ_MEMBER_PROPERTY("Up", m_vUp),
  }
  EZ_END_PROPERTIES;
}
EZ_END_STATIC_REFLECTED_TYPE;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAudioSystemSoundObstructionType, 1)
  EZ_ENUM_CONSTANT(ezAudioSystemSoundObstructionType::None),
  EZ_ENUM_CONSTANT(ezAudioSystemSoundObstructionType::SingleRay),
  EZ_ENUM_CONSTANT(ezAudioSystemSoundObstructionType::MultipleRay),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAudioSystemControlType, 1)
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::Invalid),
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::Trigger),
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::Rtpc),
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::SoundBank),
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::Switch),
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::SwitchState),
  EZ_ENUM_CONSTANT(ezAudioSystemControlType::Environment),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAudioSystemTriggerState, 1)
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Invalid),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Playing),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Ready),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Loading),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Unloading),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Starting),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Stopping),
  EZ_ENUM_CONSTANT(ezAudioSystemTriggerState::Stopped),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemEntityData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemListenerData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemTriggerData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemRtpcData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioSystemEventData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_AudioSystemData);
