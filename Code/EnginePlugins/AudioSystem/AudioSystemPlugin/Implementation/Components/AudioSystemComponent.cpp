#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioProxyComponent.h>
#include <AudioSystemPlugin/Components/AudioSystemComponent.h>

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
      m_pProxyComponent->m_uiRefCount++;
    }
  }
}

void ezAudioSystemProxyDependentComponent::Deinitialize()
{
  if (m_pProxyComponent != nullptr)
  {
    m_pProxyComponent->m_uiRefCount--;

    if (m_pProxyComponent->m_uiRefCount == 0)
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

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioSystemComponent);
