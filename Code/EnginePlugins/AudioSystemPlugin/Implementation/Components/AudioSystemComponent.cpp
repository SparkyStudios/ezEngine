#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioProxyComponent.h>
#include <AudioSystemPlugin/Components/AudioSystemComponent.h>
#include <AudioSystemPlugin/Core/AudioWorldModule.h>

// clang-format off
EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(ezAudioSystemComponent, 1)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Sound"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_ABSTRACT_COMPONENT_TYPE;

EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(ezAudioSystemProxyDependentComponent, 1)
EZ_END_ABSTRACT_COMPONENT_TYPE;

EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(ezAudioSystemEnvironmentComponent, 1)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezSphereManipulatorAttribute("MaxDistance"),
  }
  EZ_END_ATTRIBUTES;

  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Environment", m_sEnvironmentName),
    EZ_ACCESSOR_PROPERTY("MaxDistance", GetMaxDistance, SetMaxDistance)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, ezVariant()), new ezSuffixAttribute(" m")),
    EZ_MEMBER_PROPERTY("Color", m_ShapeColor),
  }
  EZ_END_PROPERTIES;
}
EZ_END_ABSTRACT_COMPONENT_TYPE;
// clang-format on

void ezAudioSystemProxyDependentComponent::Initialize()
{
  SUPER::Initialize();

  if (m_pProxyComponent == nullptr)
  {
    GetOwner()->TryGetComponentOfBaseType(m_pProxyComponent);
    if (m_pProxyComponent == nullptr)
    {
      GetOwner()->GetWorld()->GetOrCreateComponentManager<ezAudioProxyComponentManager>()->CreateComponent(GetOwner(), m_pProxyComponent);
      if (m_pProxyComponent == nullptr)
      {
        ezLog::Error("Unable to create an Audio Proxy component on GameObject {0}", GetOwner()->GetName());
      }
    }

    if (m_pProxyComponent != nullptr)
    {
      m_pProxyComponent->AddRef();
    }
  }
}

void ezAudioSystemProxyDependentComponent::Deinitialize()
{
  if (m_pProxyComponent != nullptr)
  {
    m_pProxyComponent->ReleaseRef();

    if (!m_pProxyComponent->IsReferenced())
    {
      GetOwner()->GetWorld()->GetOrCreateComponentManager<ezAudioProxyComponentManager>()->DeleteComponent(m_pProxyComponent);
    }

    m_pProxyComponent = nullptr;
  }

  SUPER::Deinitialize();
}

ezAudioSystemDataID ezAudioSystemProxyDependentComponent::GetEntityId() const
{
  if (m_pProxyComponent == nullptr)
    return kInvalidAudioSystemId;

  return m_pProxyComponent->GetEntityId();
}

ezAudioSystemEnvironmentComponent::ezAudioSystemEnvironmentComponent()
  : ezAudioSystemProxyDependentComponent()
  , m_fMaxDistance(1)
{
}

void ezAudioSystemEnvironmentComponent::OnActivated()
{
  SUPER::OnActivated();

  GetWorld()->GetOrCreateModule<ezAudioWorldModule>()->AddEnvironment(this);
}

void ezAudioSystemEnvironmentComponent::OnDeactivated()
{
  GetWorld()->GetOrCreateModule<ezAudioWorldModule>()->RemoveEnvironment(this);

  SUPER::OnDeactivated();
}

float ezAudioSystemEnvironmentComponent::GetMaxDistance() const
{
  return m_fMaxDistance;
}

ezAudioSystemDataID ezAudioSystemEnvironmentComponent::GetEnvironmentId() const
{
  return ezAudioSystem::GetSingleton()->GetEnvironmentId(m_sEnvironmentName);
}

void ezAudioSystemEnvironmentComponent::SetMaxDistance(float fFadeDistance)
{
  m_fMaxDistance = fFadeDistance;
}


EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioSystemComponent);
