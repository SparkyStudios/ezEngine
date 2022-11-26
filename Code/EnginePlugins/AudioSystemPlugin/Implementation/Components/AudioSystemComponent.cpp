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

  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgAudioSystemSetEnvironmentAmount, OnSetAmount),
  }
  EZ_END_MESSAGEHANDLERS;
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

  if (m_pProxyComponent != nullptr)
  {
    m_pProxyComponent->EnsureInitialized();
  }
}

void ezAudioSystemProxyDependentComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  if (m_pProxyComponent != nullptr)
  {
    m_pProxyComponent->EnsureSimulationStarted();
  }
}

void ezAudioSystemProxyDependentComponent::Deinitialize()
{
  if (m_pProxyComponent != nullptr)
  {
    m_pProxyComponent->ReleaseRef();

    if (!m_pProxyComponent->IsReferenced())
    {
      m_pProxyComponent->Unregister(true);
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

void ezAudioSystemEnvironmentComponent::OverrideEnvironmentAmount(float fValue)
{
  m_bOverrideValue = fValue >= 0;
  m_fOverrideValue = m_bOverrideValue ? fValue : m_fOverrideValue;
}

void ezAudioSystemEnvironmentComponent::OnSetAmount(ezMsgAudioSystemSetEnvironmentAmount& msg)
{
  OverrideEnvironmentAmount(msg.m_fAmount);

  if (!m_bOverrideValue)
    return;

  ezAudioSystemRequestSetEnvironmentAmount request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = GetEnvironmentId();
  request.m_fAmount = m_fOverrideValue;

  if (msg.m_bSync)
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  else
    ezAudioSystem::GetSingleton()->SendRequest(request);
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioSystemComponent);
