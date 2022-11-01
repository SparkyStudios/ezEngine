#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioProxyComponent.h>
#include <AudioSystemPlugin/Components/AudioSphereEnvironmentComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

constexpr ezTypeVersion kVersion_AudioSphereEnvironmentComponent = 1;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioSphereEnvironmentComponent, kVersion_AudioSphereEnvironmentComponent, ezComponentMode::Static)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezSphereManipulatorAttribute("Radius"),
    new ezSphereVisualizerAttribute("Radius", ezColor::White, "Color"),
    new ezSphereVisualizerAttribute("MaxDistance", ezColor::White, "Color"),
  }
  EZ_END_ATTRIBUTES;

  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Radius", GetRadius, SetRadius)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.1f, ezVariant()), new ezSuffixAttribute(" m")),
  }
  EZ_END_PROPERTIES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezAudioSphereEnvironmentComponent::Initialize()
{
  SUPER::Initialize();

  m_Sphere.m_vCenter = GetOwner()->GetGlobalPosition();
}

void ezAudioSphereEnvironmentComponent::Deinitialize()
{
  SUPER::Deinitialize();
}

void ezAudioSphereEnvironmentComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioSphereEnvironmentComponent);

  s << m_sEnvironmentName;
  s << m_Sphere.m_fRadius;
  s << m_fMaxDistance;
}

void ezAudioSphereEnvironmentComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioSphereEnvironmentComponent);

  s >> m_sEnvironmentName;
  s >> m_Sphere.m_fRadius;
  s >> m_fMaxDistance;
}

ezAudioSphereEnvironmentComponent::ezAudioSphereEnvironmentComponent()
  : ezAudioSystemEnvironmentComponent()
{
}

float ezAudioSphereEnvironmentComponent::GetRadius() const
{
  return m_Sphere.m_fRadius;
}

void ezAudioSphereEnvironmentComponent::SetRadius(float fRadius)
{
  m_Sphere.m_fRadius = fRadius;
}

ezAudioSystemDataID ezAudioSphereEnvironmentComponent::GetEnvironmentId() const
{
  return ezAudioSystem::GetSingleton()->GetEnvironmentId(m_sEnvironmentName);
}

float ezAudioSphereEnvironmentComponent::GetEnvironmentAmount(ezAudioProxyComponent* pProxyComponent) const
{
  const ezVec3& proxyPosition = pProxyComponent->GetOwner()->GetGlobalPosition();
  const float fDistanceToOrigin = (proxyPosition - m_Sphere.m_vCenter).GetLength();

  if (fDistanceToOrigin <= m_fMaxDistance)
    return 1.0f;

  if (!m_Sphere.Contains(proxyPosition))
    return 0.0f;

  return ezMath::Lerp(1.0f, 0.0f, (fDistanceToOrigin - m_fMaxDistance) / (m_Sphere.m_fRadius - m_fMaxDistance));
}

void ezAudioSphereEnvironmentComponent::Update()
{
  m_Sphere.m_vCenter = GetOwner()->GetGlobalPosition();
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioSphereEnvironmentComponent);
