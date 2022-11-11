#pragma once

#include <AudioSystemPlugin/AudioSystemPluginDLL.h>

#include <Foundation/Math/Vec3.h>

/// \brief Stores the transformation data for an audio entity.
struct EZ_AUDIOSYSTEMPLUGIN_DLL ezAudioEntityTransform
{
  /// \brief The position of the entity in world space.
  ezVec3 m_vPosition;

  /// \brief The velocity of the entity.
  ezVec3 m_vVelocity;

  /// \brief The forward direction of the entity in world space.
  ezVec3 m_vForward;

  /// \brief The up direction of the entity in world space.
  ezVec3 m_vUp;
};
