#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Types.h>

using ezAudioSystemDataID = ezUInt64;
using ezAudioSystemControlID = ezUInt64;

/// \brief Represents the value which marks an audio system data as invalid.
/// Valid audio system data should be greater than this number.
constexpr ezAudioSystemDataID kInvalidAudioSystemId = 0;

/// \brief Stores the transformation data for an audio entity.
struct EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemTransform
{
  /// \brief The position of the entity in world space.
  ezVec3 m_vPosition{0, 0, 0};

  /// \brief The velocity of the entity.
  ezVec3 m_vVelocity{0, 0, 0};

  /// \brief The forward direction of the entity in world space.
  ezVec3 m_vForward{0, 0, 0};

  /// \brief The up direction of the entity in world space.
  ezVec3 m_vUp{0, 0, 0};

  bool operator==(const ezAudioSystemTransform& rhs) const
  {
    return m_vPosition == rhs.m_vPosition && m_vForward == rhs.m_vForward && m_vUp == rhs.m_vUp && m_vVelocity == rhs.m_vVelocity;
  }

  bool operator!=(const ezAudioSystemTransform& rhs) const
  {
    return !(*this == rhs);
  }
};

template <>
struct ezHashHelper<ezAudioSystemTransform>
{
  EZ_ALWAYS_INLINE static ezUInt32 Hash(const ezAudioSystemTransform& value)
  {
    return ezHashingUtils::CombineHashValues32(
      ezHashHelper<ezUInt64>::Hash(ezMath::FloatToInt(value.m_vForward.x * value.m_vForward.y * value.m_vForward.z)),
      ezHashingUtils::CombineHashValues32(
        ezHashHelper<ezUInt64>::Hash(ezMath::FloatToInt(value.m_vPosition.x * value.m_vPosition.y * value.m_vPosition.z)),
        ezHashingUtils::CombineHashValues32(
          ezHashHelper<ezUInt64>::Hash(ezMath::FloatToInt(value.m_vUp.x * value.m_vUp.y * value.m_vUp.z)),
          ezHashHelper<ezUInt64>::Hash(ezMath::FloatToInt(value.m_vVelocity.x * value.m_vVelocity.y * value.m_vVelocity.z)))));
  }

  EZ_ALWAYS_INLINE static bool Equal(const ezAudioSystemTransform& a, const ezAudioSystemTransform& b)
  {
    return a == b;
  }
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_AUDIOSYSTEMPLUGIN_DLL, ezAudioSystemTransform);

/// \brief The obstruction type applied to a sound. This affects the way that
/// ray casting works for an audio source.
struct EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemSoundObstructionType
{
  using StorageType = ezUInt8;

  enum Enum : StorageType
  {
    /// \brief No ray casting is done for this sound.
    /// The sound is neither obstructed nor occluded.
    None = 0,

    /// \brief Only one ray is shot at each frame.
    /// The occlusion value will be averaged over time.
    /// The sound will not be obstructed, since only one ray is not enough
    /// to compute this information.
    SingleRay,

    /// \brief Multiple rays are shot at each frame.
    /// The occlusion and obstructions values will be averaged over time.
    MultipleRay,

    Default = SingleRay
  };

  EZ_ENUM_TO_STRING(None, SingleRay, MultipleRay);
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_AUDIOSYSTEMPLUGIN_DLL, ezAudioSystemSoundObstructionType);

/// \brief The state of an audio trigger.
struct EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemTriggerState
{
  using StorageType = ezUInt8;

  enum Enum : StorageType
  {
    /// \brief The trigger have no state, this means it has not been loaded yet
    /// by the audio middleware.
    Invalid = 0,

    /// \brief The trigger is activated and currently playing an event.
    Playing = 1,

    /// \brief The trigger is ready to be activated. This state is set after the
    /// trigger is loaded through LoadTrigger.
    Ready = 2,

    /// \brief The trigger is being loaded.
    Loading = 3,

    /// \brief The trigger is being unloaded.
    Unloading = 4,

    /// \brief The trigger is being activated.
    Starting = 5,

    /// \brief The trigger is being stopped.
    Stopping = 6,

    /// \brief The trigger is stopped, and not playing an event.
    Stopped = 7,

    Default = Invalid,
  };

  EZ_ENUM_TO_STRING(Invalid, Playing, Ready, Loading, Unloading, Starting, Stopping, Stopped);
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_AUDIOSYSTEMPLUGIN_DLL, ezAudioSystemTriggerState);

/// \brief The state of an audio source.
enum class ezAudioSystemEventState : ezUInt8
{
  /// \brief The event have no state, this means it has not been loaded yet,
  /// nor triggered by any trigger.
  Invalid = 0,

  /// \brief The event is currently playing audio.
  Playing = 1,

  /// \brief The event is loading.
  Loading = 2,

  /// \brief The event is being unloaded.
  Unloading = 3,
};

/// \brief Base class for an audio middleware entity.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemEntityData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemEntityData, ezReflectedClass);

public:
  ~ezAudioSystemEntityData() override = default;
};

/// \brief Base class for an audio middleware listener.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemListenerData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemListenerData, ezReflectedClass);

public:
  ~ezAudioSystemListenerData() override = default;
};

/// \brief Base class for an audio middleware trigger.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemTriggerData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemTriggerData, ezReflectedClass);

public:
  ~ezAudioSystemTriggerData() override = default;
};

/// \brief Base class for an audio middleware RTPC.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemRtpcData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemRtpcData, ezReflectedClass);

public:
  ~ezAudioSystemRtpcData() override = default;
};

/// \brief Base class for an audio middleware switch state.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemSwitchStateData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemSwitchStateData, ezReflectedClass);

public:
  ~ezAudioSystemSwitchStateData() override = default;
};

/// \brief Base class for an audio middleware environment.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemEnvironmentData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemEnvironmentData, ezReflectedClass);

public:
  ~ezAudioSystemEnvironmentData() override = default;
};

/// \brief Base class for an audio middleware event.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemEventData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemEventData, ezReflectedClass);

  ~ezAudioSystemEventData() override = default;
};

/// \brief Base class for an audio middleware source.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemSourceData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemSourceData, ezReflectedClass);

public:
  ~ezAudioSystemSourceData() override = default;
};

/// \brief Base class for an audio middleware bank file.
class EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioSystemBankData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioSystemBankData, ezReflectedClass);

public:
  ~ezAudioSystemBankData() override = default;
};
