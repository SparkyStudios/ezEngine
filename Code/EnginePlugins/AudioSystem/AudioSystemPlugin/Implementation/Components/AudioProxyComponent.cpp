#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioProxyComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>
#include <AudioSystemPlugin/Core/AudioWorldModule.h>

static ezAudioSystemDataID s_uiNextEntityId = 2; // 1 is reserved for the global entity.

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioProxyComponent, 1, ezComponentMode::Static)
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezAudioProxyComponent::Initialize()
{
  SUPER::Initialize();

  ezAudioSystemRequestRegisterEntity request;

  request.m_uiEntityId = m_uiEntityId;
  request.m_sName = GetOwner()->GetName();

  ezAudioSystem::GetSingleton()->SendRequestSync(request);

  ezLog::Info("Audio Proxy Component Initialized ({0})", m_uiEntityId);
}

void ezAudioProxyComponent::Deinitialize()
{
  ezAudioSystemRequestUnregisterEntity request;

  request.m_uiEntityId = m_uiEntityId;

  ezAudioSystem::GetSingleton()->SendRequest(request);

  SUPER::Deinitialize();
}

ezAudioProxyComponent::ezAudioProxyComponent()
  : m_uiEntityId(s_uiNextEntityId++)
{
}

ezAudioProxyComponent::~ezAudioProxyComponent() = default;

ezAudioSystemDataID ezAudioProxyComponent::GetEntityId() const
{
  return m_uiEntityId;
}

void ezAudioProxyComponent::Update()
{
  // Position update
  {
    const auto& rotation = GetOwner()->GetGlobalRotation();

    ezAudioSystemTransform transform;
    transform.m_vPosition = GetOwner()->GetGlobalPosition();
    transform.m_vForward = (rotation * ezVec3::UnitXAxis()).GetNormalized();
    transform.m_vUp = (rotation * ezVec3::UnitZAxis()).GetNormalized();
    transform.m_vVelocity = GetOwner()->GetVelocity();

    if (transform == m_LastTransform)
      return;

    ezAudioSystemRequestSetEntityTransform request;

    request.m_uiEntityId = m_uiEntityId;
    request.m_Transform = transform;

    request.m_Callback = [this](const ezAudioSystemRequestSetEntityTransform& m)
    {
      if (m.m_eStatus.Failed())
        return;

      m_LastTransform = m.m_Transform;
    };

    ezAudioSystem::GetSingleton()->SendRequest(request);
  }

  ezAudioSystemRequestsQueue rq;

  // Collect environments amounts
  {
    const auto* pAudioWorldModule = GetWorld()->GetOrCreateModule<ezAudioWorldModule>();
    if (pAudioWorldModule == nullptr)
      return;

    for (auto it = pAudioWorldModule->GetEnvironments(); it.IsValid(); ++it)
    {
      const auto& pComponent = it.Key();
      if (!pComponent->IsActiveAndInitialized())
        continue;

      const ezAudioSystemDataID id = pComponent->GetEnvironmentId();
      if (id == kInvalidAudioSystemId)
        continue;

      m_mEnvironmentAmounts[id].m_fNextAmount = pComponent->GetEnvironmentAmount(this);

      if (ezMath::Abs(m_mEnvironmentAmounts[id].m_fNextAmount - m_mEnvironmentAmounts[id].m_fPreviousAmount) < ezMath::DefaultEpsilon<float>())
        continue;

      ezAudioSystemRequestSetEnvironmentAmount request;

      request.m_uiEntityId = m_uiEntityId;
      request.m_uiObjectId = id;
      request.m_fAmount = m_mEnvironmentAmounts[id].m_fNextAmount;

      rq.PushBack(request);
    }
  }

  // Apply environment amounts
  {
    ezAudioSystem::GetSingleton()->SendRequests(rq);
  }
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioProxyComponent);
