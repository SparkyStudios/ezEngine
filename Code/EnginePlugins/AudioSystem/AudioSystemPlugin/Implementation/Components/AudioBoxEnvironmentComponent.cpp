#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioBoxEnvironmentComponent.h>
#include <AudioSystemPlugin/Components/AudioProxyComponent.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

constexpr ezTypeVersion kVersion_AudioBoxEnvironmentComponent = 1;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioBoxEnvironmentComponent, kVersion_AudioBoxEnvironmentComponent, ezComponentMode::Static)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezBoxManipulatorAttribute("HalfExtends"),
    new ezBoxVisualizerAttribute("HalfExtends", 2.0f, ezColor::White, "Color"),
    new ezSphereVisualizerAttribute("MaxDistance", ezColor::White, "Color"),
  }
  EZ_END_ATTRIBUTES;

  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("HalfExtends", GetHalfExtends, SetHalfExtends)->AddAttributes(new ezDefaultValueAttribute(ezVec3(0.5f)), new ezClampValueAttribute(0.1f, ezVariant()), new ezSuffixAttribute(" m")),
  }
  EZ_END_PROPERTIES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on


void ezAudioBoxEnvironmentComponent::Initialize()
{
  SUPER::Initialize();

  m_Box.SetCenterAndHalfExtents(GetOwner()->GetGlobalPosition(), m_vHalfExtends);
}

void ezAudioBoxEnvironmentComponent::Deinitialize()
{
  SUPER::Deinitialize();
}

void ezAudioBoxEnvironmentComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioBoxEnvironmentComponent);

  s << m_sEnvironmentName;
  s << m_vHalfExtends;
  s << m_fMaxDistance;
}

void ezAudioBoxEnvironmentComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioBoxEnvironmentComponent);

  s >> m_sEnvironmentName;
  s >> m_vHalfExtends;
  s >> m_fMaxDistance;
}

float ezAudioBoxEnvironmentComponent::GetEnvironmentAmount(ezAudioProxyComponent* pProxyComponent) const
{
  const ezVec3& proxyPosition = pProxyComponent->GetOwner()->GetGlobalPosition();
  const ezVec3& boxCenter = m_Box.GetCenter();

  if (m_Box.Contains(proxyPosition))
    return 1.0f;

  const ezVec3& direction = (proxyPosition - boxCenter);
  const float fDistanceToOrigin = direction.GetLength();

  if (fDistanceToOrigin >= m_fMaxDistance)
    return 0.0f;

  ezVec3 startPoint;
  m_Box.GetRayIntersection(boxCenter, direction, nullptr, &startPoint);
  const float fDistanceToBox = (startPoint - boxCenter).GetLength();

  return ezMath::Lerp(1.0f, 0.0f, (fDistanceToOrigin - m_fMaxDistance) / (fDistanceToBox - m_fMaxDistance));
}

const ezVec3& ezAudioBoxEnvironmentComponent::GetHalfExtends() const
{
  return m_vHalfExtends;
}

void ezAudioBoxEnvironmentComponent::SetHalfExtends(const ezVec3& vHalfExtends)
{
  m_vHalfExtends = vHalfExtends;
}

void ezAudioBoxEnvironmentComponent::Update()
{
  m_Box.SetCenterAndHalfExtents(GetOwner()->GetGlobalPosition(), m_vHalfExtends);
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioBoxEnvironmentComponent);
