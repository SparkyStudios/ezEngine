#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioListenerComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>
#include <AudioSystemPlugin/Core/AudioWorldModule.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

constexpr ezTypeVersion kVersion_AudioListenerComponent = 1;

static ezUInt32 s_uiNextListenerId = 2; // 1 is reserved for the default listener

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioListenerComponent, kVersion_AudioListenerComponent, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("PositionGameObject", _DoNotCall, SetListenerPositionObject)->AddAttributes(new ezGameObjectReferenceAttribute()),
    EZ_ACCESSOR_PROPERTY("RotationGameObject", _DoNotCall, SetListenerOrientationObject)->AddAttributes(new ezGameObjectReferenceAttribute()),
    EZ_ACCESSOR_PROPERTY("IsDefault", IsDefault, SetDefault)->AddAttributes(new ezDefaultValueAttribute(false)),
    EZ_MEMBER_PROPERTY("PositionOffset", m_vListenerPositionOffset),
  }
  EZ_END_PROPERTIES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezAudioListenerComponent::OnActivated()
{
  SUPER::OnActivated();

  if (auto* const audioWorldModule = GetWorld()->GetOrCreateModule<ezAudioWorldModule>(); audioWorldModule->GetDefaultListener() != this && m_bIsDefault)
  {
    audioWorldModule->SetDefaultListener(this);
  }

  ezAudioSystemRequestRegisterListener request;

  request.m_uiListenerId = m_uiListenerId;
  request.m_sName = GetOwner()->GetName();

  // Prefer to send this request synchronously...
  ezAudioSystem::GetSingleton()->SendRequestSync(request);
}

void ezAudioListenerComponent::OnDeactivated()
{
  ezAudioSystemRequestUnregisterListener request;

  request.m_uiListenerId = m_uiListenerId;

  // Prefer to send this request synchronously...
  ezAudioSystem::GetSingleton()->SendRequestSync(request);

  if (auto* const audioWorldModule = GetWorld()->GetOrCreateModule<ezAudioWorldModule>(); audioWorldModule->GetDefaultListener() == this)
  {
    audioWorldModule->SetDefaultListener(nullptr);
  }

  SUPER::OnDeactivated();
}

void ezAudioListenerComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioListenerComponent);

  stream.WriteGameObjectHandle(m_hListenerPositionObject);
  stream.WriteGameObjectHandle(m_hListenerRotationObject);
  s << m_vListenerPositionOffset;
}

void ezAudioListenerComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioListenerComponent);

  m_hListenerPositionObject = stream.ReadGameObjectHandle();
  m_hListenerRotationObject = stream.ReadGameObjectHandle();
  s >> m_vListenerPositionOffset;
}

ezAudioListenerComponent::ezAudioListenerComponent()
  : ezAudioSystemComponent()
  , m_uiListenerId(s_uiNextListenerId++)
  , m_bIsDefault(false)
{
}

ezAudioListenerComponent::~ezAudioListenerComponent() = default;

void ezAudioListenerComponent::SetListenerPositionObject(const char* szGuid)
{
  const auto& resolver = GetWorld()->GetGameObjectReferenceResolver();

  if (!resolver.IsValid())
    return;

  m_hListenerPositionObject = resolver(szGuid, GetHandle(), "PositionGameObject");
}

void ezAudioListenerComponent::SetListenerOrientationObject(const char* szGuid)
{
  const auto& resolver = GetWorld()->GetGameObjectReferenceResolver();

  if (!resolver.IsValid())
    return;

  m_hListenerRotationObject = resolver(szGuid, GetHandle(), "RotationGameObject");
}

void ezAudioListenerComponent::SetDefault(bool bDefault)
{
  m_bIsDefault = bDefault;

  auto* const audioWorldModule = GetWorld()->GetOrCreateModule<ezAudioWorldModule>(); 

  if (audioWorldModule->GetDefaultListener() != this && m_bIsDefault)
  {
    audioWorldModule->SetDefaultListener(this);
  }

  if (audioWorldModule->GetDefaultListener() == this && !m_bIsDefault)
  {
    audioWorldModule->SetDefaultListener(nullptr);
  }
}

ezVec3 ezAudioListenerComponent::GetListenerPosition() const
{
  ezVec3 position;

  if (const ezGameObject* pObject = nullptr; GetWorld()->TryGetObject(m_hListenerPositionObject, pObject))
  {
    position = pObject->GetGlobalPosition();
  }
  else
  {
    position = GetOwner()->GetGlobalPosition();
  }

  return position + m_vListenerPositionOffset;
}

ezVec3 ezAudioListenerComponent::GetListenerVelocity() const
{
  if (const ezGameObject* pObject = nullptr; GetWorld()->TryGetObject(m_hListenerPositionObject, pObject))
  {
    return pObject->GetLinearVelocity();
  }

  return GetOwner()->GetLinearVelocity();
}

ezQuat ezAudioListenerComponent::GetListenerRotation() const
{
  if (const ezGameObject* pObject = nullptr; GetWorld()->TryGetObject(m_hListenerRotationObject, pObject))
  {
    return pObject->GetGlobalRotation();
  }

  return GetOwner()->GetGlobalRotation();
}

bool ezAudioListenerComponent::IsDefault() const
{
  return m_bIsDefault;
}

void ezAudioListenerComponent::Update()
{
  const auto& position = GetListenerPosition();
  const auto& rotation = GetListenerRotation();
  const auto& velocity = GetListenerVelocity();

  const auto& fw = (rotation * ezVec3::MakeAxisX()).GetNormalized();
  const auto& up = (rotation * ezVec3::MakeAxisZ()).GetNormalized();

  {
    ezAudioSystemTransform transform;
    transform.m_vForward = fw;
    transform.m_vPosition = position;
    transform.m_vUp = up;
    transform.m_vVelocity = velocity;

    if (m_LastTransform == transform)
      return;
  }

  ezAudioSystem::GetSingleton()->SetListener(m_uiListenerId, position, fw, up, velocity);
}

const char* ezAudioListenerComponent::_DoNotCall() const
{
  return nullptr;
}

EZ_STATICLINK_FILE(AudioSystem, AudioSystemPlugin_Implementation_Components_AudioListenerComponent);
