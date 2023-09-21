#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioAnimationComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Core/Messages/CommonMessages.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

#include <RendererCore/AnimationSystem/Declarations.h>
#include <RendererCore/AnimationSystem/Skeleton.h>

constexpr ezTypeVersion kVersion_AudioAnimationComponent = 1;
constexpr ezTypeVersion kVersion_AudioAnimationEntry = 1;

/// \brief The last used event ID for all audio triggers.
extern ezAudioSystemDataID s_uiNextEventId;
extern ezAudioSystemDataID s_uiNextEntityId;

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioAnimationEntry, kVersion_AudioAnimationEntry, ezRTTIDefaultAllocator<ezAudioAnimationEntry>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Event", m_sEventName),
    EZ_MEMBER_PROPERTY("Trigger", m_sTriggerName),
    EZ_ACCESSOR_PROPERTY("Joint", GetJointName, SetJointName),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezAudioAnimationComponent, kVersion_AudioAnimationComponent, ezComponentMode::Static)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Sound"),
  }
  EZ_END_ATTRIBUTES;

  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("Events", m_EventEntries),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgAnimationPoseUpdated, OnAnimationPoseUpdated),
    EZ_MESSAGE_HANDLER(ezMsgGenericEvent, OnAnimationEvent),
  }
  EZ_END_MESSAGEHANDLERS;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

ezAudioAnimationEntry::ezAudioAnimationEntry()
  : m_uiEntityId(s_uiNextEntityId++)
  , m_uiEventId(s_uiNextEventId++)
  , m_uiJointIndex(ezInvalidJointIndex)
  , m_bTriggerLoaded(false)
{
}

void ezAudioAnimationEntry::SetJointName(const char* szName)
{
  m_sJointName.Assign(szName);
  m_uiJointIndex = ezInvalidJointIndex;
}

const char* ezAudioAnimationEntry::GetJointName() const
{
  return m_sJointName.GetData();
}

void ezAudioAnimationEntry::ActivateTrigger() const
{
  ezAudioSystemRequestActivateTrigger request;

  request.m_uiEntityId = m_uiEntityId;
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sTriggerName);
  request.m_uiEventId = m_uiEventId;

  ezAudioSystem::GetSingleton()->SendRequest(request);
}

void ezAudioAnimationEntry::Initialize(bool bSync)
{
  if (m_bTriggerLoaded)
    return;

  if (m_sTriggerName.IsEmpty())
    return;

  ezAudioSystemRequestRegisterEntity registerEntity;

  {
    ezStringBuilder name;
    name.Format("AudioAnimation Entity: {}", m_sEventName);

    registerEntity.m_uiEntityId = m_uiEntityId;
    registerEntity.m_sName = name;
  }

  ezAudioSystemRequestLoadTrigger loadTrigger;

  loadTrigger.m_uiEntityId = m_uiEntityId;
  loadTrigger.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sTriggerName);
  loadTrigger.m_uiEventId = m_uiEventId;

  loadTrigger.m_Callback = [this](const ezAudioSystemRequestLoadTrigger& m)
  {
    if (m.m_eStatus.Failed())
    {
      return;
    }

    m_bTriggerLoaded = true;
    ezLog::Debug("[AudioSystem] Loaded Trigger '{0}'.", m_sTriggerName);
  };

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(registerEntity);
    ezAudioSystem::GetSingleton()->SendRequestSync(loadTrigger);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(registerEntity);
    ezAudioSystem::GetSingleton()->SendRequest(loadTrigger);
  }
}

void ezAudioAnimationEntry::UnloadTrigger()
{
  if (m_bTriggerLoaded)
    return;

  if (m_sTriggerName.IsEmpty())
    return;

  ezAudioSystemRequestUnloadTrigger request;

  request.m_uiEntityId = m_uiEntityId;
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sTriggerName);

  request.m_Callback = [this](const ezAudioSystemRequestUnloadTrigger& m)
  {
    if (m.m_eStatus.Failed())
      return;

    m_bTriggerLoaded = false;
    ezLog::Debug("[AudioSystem] Unloaded Trigger '{0}'.", m_sTriggerName);
  };

  ezAudioSystem::GetSingleton()->SendRequest(request);
}

void ezAudioAnimationComponent::Initialize()
{
  SUPER::Initialize();

  for (auto& entry : m_EventEntries)
  {
    entry.Initialize(false);
  }
}

void ezAudioAnimationComponent::Deinitialize()
{
  for (auto& entry : m_EventEntries)
  {
    entry.UnloadTrigger();
  }

  SUPER::Deinitialize();
}

void ezAudioAnimationComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioAnimationComponent);

  s << m_EventEntries.GetCount();
  for (auto& entry : m_EventEntries)
  {
    s.WriteVersion(kVersion_AudioAnimationEntry);
    s << entry.m_sEventName;
    s << entry.m_sTriggerName;
    s << entry.m_sJointName;
  }
}

void ezAudioAnimationComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioAnimationComponent);

  ezUInt32 count;
  s >> count;

  if (count == 0)
    return;

  m_EventEntries.Reserve(count);
  for (ezUInt32 i = 0; i < count; i++)
  {
    ezAudioAnimationEntry entry{};
    s.ReadVersion(kVersion_AudioAnimationEntry);
    s >> entry.m_sEventName;
    s >> entry.m_sTriggerName;
    s >> entry.m_sJointName;

    m_EventEntries.PushBack(entry);
  }
}

void ezAudioAnimationComponent::Update()
{
  ezAudioSystemRequestsQueue queue;

  for (auto& entry : m_EventEntries)
  {
    if (entry.m_sJointName.IsEmpty())
    {
      const auto& rotation = GetOwner()->GetGlobalRotation();

      ezAudioSystemTransform transform;
      transform.m_vPosition = GetOwner()->GetGlobalPosition();
      transform.m_vForward = (rotation * ezVec3::MakeAxisX()).GetNormalized();
      transform.m_vUp = (rotation * ezVec3::MakeAxisZ()).GetNormalized();
      transform.m_vVelocity = GetOwner()->GetLinearVelocity();

      if (transform == entry.m_LastTransform)
        return;

      ezAudioSystemRequestSetEntityTransform request;

      request.m_uiEntityId = entry.m_uiEntityId;
      request.m_Transform = transform;

      request.m_Callback = [&entry](const ezAudioSystemRequestSetEntityTransform& m)
      {
        if (m.m_eStatus.Failed())
          return;

        entry.m_LastTransform = m.m_Transform;
      };

      queue.PushBack(request);
    }
  }

  ezAudioSystem::GetSingleton()->SendRequests(queue);
}

void ezAudioAnimationComponent::OnAnimationPoseUpdated(ezMsgAnimationPoseUpdated& msg)
{
  ezAudioSystemRequestsQueue queue;

  for (auto& entry : m_EventEntries)
  {
    if (!entry.m_sJointName.IsEmpty() && entry.m_uiJointIndex == ezInvalidJointIndex)
    {
      entry.m_uiJointIndex = msg.m_pSkeleton->FindJointByName(entry.m_sJointName);
    }

    if (entry.m_uiJointIndex == ezInvalidJointIndex)
      continue;

    ezMat4 bone;
    ezQuat boneRot;

    msg.ComputeFullBoneTransform(entry.m_uiJointIndex, bone, boneRot);

    ezAudioSystemTransform transform;
    transform.m_vPosition = GetOwner()->GetGlobalPosition() + bone.GetTranslationVector();
    transform.m_vForward = (boneRot * ezVec3::MakeAxisX()).GetNormalized();
    transform.m_vUp = (boneRot * ezVec3::MakeAxisZ()).GetNormalized();
    transform.m_vVelocity = transform.m_vPosition - entry.m_LastTransform.m_vPosition; // We can just mimic a velocity, since we have not this data in the bone transform

    if (transform == entry.m_LastTransform)
      continue;

    ezAudioSystemRequestSetEntityTransform request;

    request.m_uiEntityId = entry.m_uiEntityId;
    request.m_Transform = transform;

    request.m_Callback = [&entry](const ezAudioSystemRequestSetEntityTransform& m)
    {
      if (m.m_eStatus.Failed())
        return;

      entry.m_LastTransform = m.m_Transform;
    };

    queue.PushBack(request);
  }

  ezAudioSystem::GetSingleton()->SendRequests(queue);
}

void ezAudioAnimationComponent::OnAnimationEvent(ezMsgGenericEvent& msg) const
{
  for (auto& entry : m_EventEntries)
  {
    if (entry.m_sEventName != msg.m_sMessage)
      continue;

    if (entry.m_sTriggerName.IsEmpty())
      continue;

    if (!entry.m_sJointName.IsEmpty() && entry.m_uiJointIndex == ezInvalidJointIndex)
      continue;

    entry.ActivateTrigger();

    ezLog::Debug("[AudioSystem] Trigger '{0}' activated for event {1}.", entry.m_sTriggerName, msg.m_sMessage);
  }
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioAnimationComponent);
