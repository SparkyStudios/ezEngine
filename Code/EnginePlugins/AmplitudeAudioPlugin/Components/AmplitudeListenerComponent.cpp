#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeListenerComponent.h>
#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>

using namespace SparkyStudios::Audio::Amplitude;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAmplitudeListenerComponent, 1, ezComponentMode::Static)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_MEMBER_PROPERTY("ListenerID", m_uiListenerId),
    }
    EZ_END_PROPERTIES;
  }
EZ_END_DYNAMIC_REFLECTED_TYPE

// clang-format on

ezAmplitudeListenerComponent::ezAmplitudeListenerComponent() = default;
ezAmplitudeListenerComponent::~ezAmplitudeListenerComponent() = default;

void ezAmplitudeListenerComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s << m_uiListenerId;
}

void ezAmplitudeListenerComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s >> m_uiListenerId;
}

void ezAmplitudeListenerComponent::Update()
{
  // const auto pos = GetOwner()->GetGlobalPosition();
  // const auto vel = GetOwner()->GetVelocity();
  // const auto fwd = (GetOwner()->GetGlobalRotation() * ezVec3(1, 0, 0)).GetNormalized();
  // const auto up = (GetOwner()->GetGlobalRotation() * ezVec3(0, 0, 1)).GetNormalized();

  // ezAmplitude::GetSingleton()->SetListener(m_uiListenerId, pos, fwd, up, vel);
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AmplitudeListenerComponent);
