#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioProxyComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

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

  request.m_Callback = [](const ezAudioSystemRequestRegisterEntity& m)
  {
    if (m.m_eStatus.Failed())
      return;

    ezLog::Info("[AudioSystem] Registered entity '{0}' in the audio system.", m.m_uiEntityId);
  };

  ezAudioSystem::GetSingleton()->SendRequestSync(request);

  ezLog::Info("AudioSystem Component Initialized");
}

void ezAudioProxyComponent::Deinitialize()
{
  ezAudioSystemRequestUnregisterEntity request;

  request.m_uiEntityId = m_uiEntityId;

  request.m_Callback = [](const ezAudioSystemRequestUnregisterEntity& m)
  {
    if (m.m_eStatus.Failed())
      return;

    ezLog::Info("[AudioSystem] Unregistered entity '{0}' in the audio system.", m.m_uiEntityId);
  };

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

    ezLog::Info("[AudioSystem] Updated transform for entity '{0}'.", m.m_uiEntityId);
    m_LastTransform = m.m_Transform;
  };

  ezAudioSystem::GetSingleton()->SendRequest(request);
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioProxyComponent);
