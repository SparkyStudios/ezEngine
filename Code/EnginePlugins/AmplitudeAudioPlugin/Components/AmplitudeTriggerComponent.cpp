#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <Core/Interfaces/PhysicsWorldModule.h>
#include <Core/Messages/CommonMessages.h>
#include <Core/Messages/DeleteObjectMessage.h>
#include <Core/ResourceManager/Resource.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

#include <Core/Interfaces/PhysicsWorldModule.h>
#include <Foundation/Configuration/CVar.h>
#include <GameEngine/VisualScript/VisualScriptInstance.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>
#include <AmplitudeAudioPlugin/Components/AmplitudeTriggerComponent.h>

using namespace SparkyStudios::Audio;

ezCVarInt cvar_AmplitudeOcclusionNumRays("Amplitude.Occlusion.NumRays", 2, ezCVarFlags::Default, "Number of occlusion rays per component per frame");

static ezVec3 s_InSpherePositions[32];
static bool s_bInSpherePositionsInitialized = false;

// clang-format off
EZ_IMPLEMENT_MESSAGE_TYPE(ezMsgAmplitudeSoundEnded);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezMsgAmplitudeSoundEnded, 1, ezRTTIDefaultAllocator<ezMsgAmplitudeSoundEnded>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAmplitudeTriggerComponent, 1, ezComponentMode::Static)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_ACCESSOR_PROPERTY("Gain", GetGain, SetGain)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 1.0f)),
      EZ_ACCESSOR_PROPERTY("SoundObjectName", GetSoundObjectName, SetSoundObjectName),
      EZ_ACCESSOR_PROPERTY("OcclusionCollisionLayer", GetOcclusionCollisionLayer, SetOcclusionCollisionLayer)->AddAttributes(new ezDynamicEnumAttribute("PhysicsCollisionLayer")),
      EZ_ENUM_MEMBER_PROPERTY("OnFinishedAction", ezOnComponentFinishedAction, m_OnFinishedAction),
    }
    EZ_END_PROPERTIES;

    EZ_BEGIN_MESSAGEHANDLERS
    {
      EZ_MESSAGE_HANDLER(ezMsgDeleteGameObject, OnMsgDeleteGameObject),
    }
    EZ_END_MESSAGEHANDLERS;

    EZ_BEGIN_MESSAGESENDERS
    {
      EZ_MESSAGE_SENDER(m_SoundEndedEventSender),
    }
    EZ_END_MESSAGESENDERS;

    EZ_BEGIN_FUNCTIONS
    {
      EZ_SCRIPT_FUNCTION_PROPERTY(Play),
      EZ_SCRIPT_FUNCTION_PROPERTY(Pause, In, "FadeDuration"),
      EZ_SCRIPT_FUNCTION_PROPERTY(Stop, In, "FadeDuration"),
      EZ_SCRIPT_FUNCTION_PROPERTY(Resume, In, "FadeDuration"),
    }
    EZ_END_FUNCTIONS;
  }
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

struct ezAmplitudeTriggerComponentManager::ObstructionOcclusionState
{
  ezAmplitudeTriggerComponent* m_pComponent;

  ezUInt32 m_uiRaycastHits = 0;
  ezUInt8 m_uiNextRayIndex = 0;
  ezUInt8 m_uiNumUsedRays = 0;

  float m_fLastObstructionValue = 0.0f;
  float m_fLastOcclusionValue = 0.0f;

  float GetObstructionValue(float fThreshold) const { return ezMath::Clamp((m_fLastObstructionValue - fThreshold) / ezMath::Max(1.0f - fThreshold, 0.0001f), 0.0f, 1.0f); }
  float GetOcclusionValue(float fThreshold) const { return ezMath::Clamp((m_fLastOcclusionValue - fThreshold) / ezMath::Max(1.0f - fThreshold, 0.0001f), 0.0f, 1.0f); }
};

ezAmplitudeTriggerComponentManager::ezAmplitudeTriggerComponentManager(ezWorld* pWorld)
  : ezComponentManager(pWorld)
{
  if (!s_bInSpherePositionsInitialized)
  {
    s_bInSpherePositionsInitialized = true;

    ezRandom rng;
    rng.Initialize(3);

    ezRandomGauss rngGauss;
    rngGauss.Initialize(27, 0xFFFF);


    for (auto& pos : s_InSpherePositions)
    {
      pos.x = static_cast<float>(rngGauss.SignedValue());
      pos.y = static_cast<float>(rngGauss.SignedValue());
      pos.z = static_cast<float>(rngGauss.SignedValue());

      float fRadius = ezMath::Pow(static_cast<float>(rng.DoubleZeroToOneExclusive()), 1.0f / 3.0f);
      fRadius = fRadius * 0.5f + 0.5f;
      pos.SetLength(fRadius).IgnoreResult();
    }
  }
}


void ezAmplitudeTriggerComponentManager::Initialize()
{
  SUPER::Initialize();

  {
    auto desc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezAmplitudeTriggerComponentManager::UpdateOcclusion, this);
    desc.m_Phase = ezWorldModule::UpdateFunctionDesc::Phase::Async;
    desc.m_bOnlyUpdateWhenSimulating = true;

    this->RegisterUpdateFunction(desc);
  }

  {
    auto desc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezAmplitudeTriggerComponentManager::UpdateTriggers, this);
    desc.m_Phase = ezWorldModule::UpdateFunctionDesc::Phase::PostTransform;
    desc.m_bOnlyUpdateWhenSimulating = true;

    this->RegisterUpdateFunction(desc);
  }

  ezResourceManager::GetResourceEvents().AddEventHandler(ezMakeDelegate(&ezAmplitudeTriggerComponentManager::ResourceEventHandler, this));
}


void ezAmplitudeTriggerComponentManager::Deinitialize()
{
  ezResourceManager::GetResourceEvents().RemoveEventHandler(ezMakeDelegate(&ezAmplitudeTriggerComponentManager::ResourceEventHandler, this));

  SUPER::Deinitialize();
}

ezUInt32 ezAmplitudeTriggerComponentManager::AddObstructionOcclusionState(ezAmplitudeTriggerComponent* pComponent)
{
  auto& occlusionState = m_ObstructionOcclusionStates.ExpandAndGetRef();
  occlusionState.m_pComponent = pComponent;

  if (const auto pPhysicsWorldModule = GetWorld()->GetModule<ezPhysicsWorldModuleInterface>())
  {
    // if (const ezVec3 listenerPos = ezAmplitude::GetSingleton()->GetListenerPosition(); !listenerPos.IsNaN())
    // {
    //   ShootOcclusionRays(occlusionState, listenerPos, 8, pPhysicsWorldModule, ezTime::Seconds(1000.0));
    // }
  }

  return m_ObstructionOcclusionStates.GetCount() - 1;
}

void ezAmplitudeTriggerComponentManager::RemoveObstructionOcclusionState(ezUInt32 uiIndex)
{
  if (uiIndex >= m_ObstructionOcclusionStates.GetCount())
    return;

  m_ObstructionOcclusionStates.RemoveAtAndSwap(uiIndex);

  if (uiIndex != m_ObstructionOcclusionStates.GetCount())
  {
    m_ObstructionOcclusionStates[uiIndex].m_pComponent->m_uiObstructionOcclusionStateIndex = uiIndex;
  }
}

void ezAmplitudeTriggerComponentManager::ShootOcclusionRays(ObstructionOcclusionState& state, ezVec3 listenerPos, ezUInt32 uiNumRays, const ezPhysicsWorldModuleInterface* pPhysicsWorldModule, ezTime deltaTime)
{
  const ezVec3 centerPos = state.m_pComponent->GetOwner()->GetGlobalPosition();
  if (centerPos.IsNaN())
    return;

  const ezUInt8 uiCollisionLayer = state.m_pComponent->m_uiOcclusionCollisionLayer;
  ezPhysicsCastResult hitResult;

  for (ezUInt32 i = 0; i < uiNumRays; ++i)
  {
    const ezUInt32 uiRayIndex = state.m_uiNextRayIndex;
    ezVec3 targetPos = centerPos + s_InSpherePositions[uiRayIndex] * (centerPos - listenerPos).GetLength();
    ezVec3 dir = targetPos - listenerPos;
    const float fDistance = dir.GetLengthAndNormalize();

    ezPhysicsQueryParameters query(uiCollisionLayer);
    query.m_bIgnoreInitialOverlap = true;
    query.m_ShapeTypes = ezPhysicsShapeType::Static | ezPhysicsShapeType::Dynamic;

    if (pPhysicsWorldModule->Raycast(hitResult, listenerPos, dir, fDistance, query))
    {
      state.m_uiRaycastHits |= (1 << uiRayIndex);
    }
    else
    {
      state.m_uiRaycastHits &= ~(1 << uiRayIndex);
    }

    state.m_uiNextRayIndex = (state.m_uiNextRayIndex + 1) % 32;
    state.m_uiNumUsedRays = ezMath::Min<ezUInt8>(state.m_uiNumUsedRays + 1, 32);
  }

  float fNewObstructionValue = static_cast<float>(ezMath::CountBits(state.m_uiRaycastHits)) / 32;
  float fNewOcclusionValue = static_cast<float>(ezMath::CountBits(state.m_uiRaycastHits)) / state.m_uiNumUsedRays;

  fNewObstructionValue = ezMath::Max(fNewObstructionValue, 0.0f);
  fNewOcclusionValue = ezMath::Max(fNewOcclusionValue, 0.0f);

  state.m_fLastObstructionValue = ezMath::Lerp(state.m_fLastObstructionValue, fNewObstructionValue, ezMath::Min(deltaTime.GetSeconds() * 8.0, 1.0));
  state.m_fLastOcclusionValue = ezMath::Lerp(state.m_fLastOcclusionValue, fNewOcclusionValue, ezMath::Min(deltaTime.GetSeconds() * 8.0, 1.0));
}

void ezAmplitudeTriggerComponentManager::UpdateOcclusion(const ezWorldModule::UpdateContext& context)
{
  if (const auto pPhysicsWorldModule = GetWorld()->GetModuleReadOnly<ezPhysicsWorldModuleInterface>())
  {
    // const ezVec3 listenerPos = ezAmplitude::GetSingleton()->GetListenerPosition();
    // const ezTime deltaTime = GetWorld()->GetClock().GetTimeDiff();
    //
    // const ezUInt32 uiNumRays = ezMath::Max<int>(cvar_AmplitudeOcclusionNumRays, 1);
    //
    // for (auto& occlusionState : m_ObstructionOcclusionStates)
    // {
    //   ShootOcclusionRays(occlusionState, listenerPos, uiNumRays, pPhysicsWorldModule, deltaTime);
    // }
  }
}

void ezAmplitudeTriggerComponentManager::UpdateTriggers(const ezWorldModule::UpdateContext& context)
{
  for (auto it = this->m_ComponentStorage.GetIterator(context.m_uiFirstComponentIndex, context.m_uiComponentCount); it.IsValid(); ++it)
  {
    if (ComponentType* pComponent = it; pComponent->IsActiveAndInitialized())
    {
      pComponent->Update();
    }
  }
}

void ezAmplitudeTriggerComponentManager::ResourceEventHandler(const ezResourceEvent& e)
{
  if (e.m_Type == ezResourceEvent::Type::ResourceContentUnloading && e.m_pResource->GetDynamicRTTI()->IsDerivedFrom<ezAmplitudeTriggerComponent>())
  {
    // ezFmodSoundEventResourceHandle hResource((ezFmodSoundEventResource*)(e.m_pResource));
    //
    // for (auto it = GetComponents(); it.IsValid(); it.Next())
    // {
    //   if (it->m_hSoundEvent == hResource)
    //   {
    //     it->InvalidateResource(true);
    //   }
    // }
  }
}


ezAmplitudeTriggerComponent::ezAmplitudeTriggerComponent()
  : m_sSoundObjectName()
  , m_uiOcclusionCollisionLayer(0)
  , m_TriggerChannel()
  , m_TriggerEntity()
{
}

ezAmplitudeTriggerComponent::~ezAmplitudeTriggerComponent() = default;

void ezAmplitudeTriggerComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s << m_sSoundObjectName;
  s << m_fGain;
  s << m_uiOcclusionCollisionLayer;

  ezOnComponentFinishedAction::StorageType type = m_OnFinishedAction;
  s << type;
}

void ezAmplitudeTriggerComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  s >> m_sSoundObjectName;
  s >> m_fGain;
  s >> m_uiOcclusionCollisionLayer;

  ezOnComponentFinishedAction::StorageType type;
  s >> type;
  m_OnFinishedAction = static_cast<ezOnComponentFinishedAction::Enum>(type);
}

void ezAmplitudeTriggerComponent::Pause(double dDuration)
{
  if (m_TriggerChannel.Valid())
    m_TriggerChannel.Pause(dDuration);
}

void ezAmplitudeTriggerComponent::Resume(double dDuration)
{
  if (m_TriggerChannel.Valid())
    m_TriggerChannel.Resume(dDuration);
}

void ezAmplitudeTriggerComponent::SetOcclusionCollisionLayer(ezUInt8 uiCollisionLayer)
{
  m_uiOcclusionCollisionLayer = uiCollisionLayer;
}

void ezAmplitudeTriggerComponent::SetGain(float fGain)
{
  m_fGain = fGain;
}

float ezAmplitudeTriggerComponent::GetGain() const
{
  return m_fGain;
}

void ezAmplitudeTriggerComponent::SetSoundObjectName(const char* szSoundObjectName)
{
  m_sSoundObjectName = szSoundObjectName;
}

const char* ezAmplitudeTriggerComponent::GetSoundObjectName() const
{
  return m_sSoundObjectName;
}

void ezAmplitudeTriggerComponent::OnSimulationStarted()
{
  ezLog::Debug("Simulation started");
  if (!m_TriggerEntity.Valid()) // TODO: Entity scoped triggers should be enabled by user
  {
    const Amplitude::AmEntityID uiEntityId = GetOwner()->GetHandle().GetInternalID().m_Data;
    // m_TriggerEntity = ezAmplitude::GetSingleton()->GetEngine()->AddEntity(uiEntityId);
    ezLog::Debug("Created entity {0}", m_TriggerEntity.GetId());
  }

  if (!m_TriggerChannel.Valid())
    Play();
}

void ezAmplitudeTriggerComponent::OnDeactivated()
{
  SUPER::OnDeactivated();

  static_cast<ezAmplitudeTriggerComponentManager*>(GetOwningManager())->RemoveObstructionOcclusionState(m_uiObstructionOcclusionStateIndex);
  m_uiObstructionOcclusionStateIndex = ezInvalidIndex;

  if (m_TriggerChannel.Valid())
  {
    // we could expose this decision as a property 'AlwaysFinish' or so
    bool bLetFinish = true;

    m_TriggerChannel.Stop();
    m_TriggerChannel.Clear();
  }
}

bool ezAmplitudeTriggerComponent::IsPlaying() const
{
  if (m_TriggerChannel.Valid())
    return m_TriggerChannel.Playing();

  return false;
}

void ezAmplitudeTriggerComponent::Play()
{
  ezLog::Debug("Play sound start");

  if (m_TriggerChannel.Valid() || !IsActiveAndSimulating())
  {
    ezLog::Debug("Invalid? {0} {1}", m_TriggerChannel.Valid(), !IsActiveAndSimulating());
    return;
  }

  if (m_TriggerEntity.Valid())
  {
    ezLog::Debug("Using entity {0}", m_TriggerEntity.GetId());
    // m_TriggerChannel = ezAmplitude::GetSingleton()->GetEngine()->Play(m_sSoundObjectName.GetData(), m_TriggerEntity);
  }
  else
  {
    ezLog::Debug("Being world scoped");
    const auto pos = GetOwner()->GetGlobalPosition();
    // m_TriggerChannel = ezAmplitude::GetSingleton()->GetEngine()->Play(m_sSoundObjectName.GetData(), AM_Vec3(pos.x, pos.y, pos.z));
  }

  if (!m_TriggerChannel.Valid())
    ezLog::Error("Cannot trigger sound, instance could not be created.");
}

void ezAmplitudeTriggerComponent::Stop(double dDuration)
{
  if (m_TriggerChannel.Valid())
  {
    m_TriggerChannel.Stop(dDuration);
  }
}

void ezAmplitudeTriggerComponent::OnMsgDeleteGameObject(ezMsgDeleteGameObject& msg)
{
  ezOnComponentFinishedAction::HandleDeleteObjectMsg(msg, m_OnFinishedAction);
}

void ezAmplitudeTriggerComponent::Update()
{
  Amplitude::ChannelPlaybackState state = Amplitude::ChannelPlaybackState::Stopped;

  if (m_TriggerChannel.Valid())
  {
    m_TriggerChannel.SetGain(m_fGain);


    UpdateEntity();
    UpdateOcclusion();

    state = m_TriggerChannel.GetPlaybackState();

    if (state == Amplitude::ChannelPlaybackState::Stopped)
    {
      ezMsgAmplitudeSoundEnded msg;
      m_SoundEndedEventSender.SendEventMessage(msg, this, GetOwner());

      ezOnComponentFinishedAction::HandleFinishedAction(this, m_OnFinishedAction);
    }
  }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (false) // TODO: Enable Debug
  {
    if (m_TriggerChannel.Valid())
    {
      bool is3D = false;
      if (is3D)
      {
        float minDistance = 0.0f;
        float maxDistance = 0.0f;

        ezDebugRenderer::DrawLineSphere(GetWorld(), ezBoundingSphere(GetOwner()->GetGlobalPosition(), minDistance), ezColor::Blue);
        ezDebugRenderer::DrawLineSphere(GetWorld(), ezBoundingSphere(GetOwner()->GetGlobalPosition(), maxDistance), ezColor::Cyan);
      }

      char path[128];

      const char* szStates[] = {"STOPPED", "PLAYING", "FADING_IN", "FADING_OUT", "SWITCHING_STATE", "PAUSED"};

      const char* szCurrentState = szStates[static_cast<ezUInt32>(state)];

      ezStringBuilder sb;
      sb.Format("{}\n{}", path, szCurrentState);

      if (true)
      {
        auto& occlusionState = static_cast<ezAmplitudeTriggerComponentManager*>(GetOwningManager())->GetObstructionOcclusionState(m_uiObstructionOcclusionStateIndex);
        sb.AppendFormat("\nOcclusion: {}", occlusionState.GetOcclusionValue(0.0f));     // TODO: Threshold
        sb.AppendFormat("\nObstruction: {}", occlusionState.GetObstructionValue(0.0f)); // TODO: Threshold

        ezVec3 centerPos = GetOwner()->GetGlobalPosition();
        // for (ezUInt32 uiRayIndex = 0; uiRayIndex < EZ_ARRAY_SIZE(s_InSpherePositions); ++uiRayIndex)
        // {
        //   ezVec3 targetPos = centerPos + s_InSpherePositions[uiRayIndex] * (ezAmplitude::GetSingleton()->GetListenerPosition() - centerPos).GetLength();
        //   ezColor color = (occlusionState.m_uiRaycastHits & (1 << uiRayIndex)) ? ezColor::Red : ezColor::Green;
        //   ezDebugRenderer::DrawCross(GetWorld(), targetPos, 0.1f, color);
        // }
      }

      ezDebugRenderer::Draw3DText(GetWorld(), sb, GetOwner()->GetGlobalPosition(), ezColor::Cyan);
    }
  }
#endif
}

void ezAmplitudeTriggerComponent::UpdateEntity()
{
  if (!m_TriggerEntity.Valid())
    return;

  const auto pos = GetOwner()->GetGlobalPosition();
  const auto fwd = (GetOwner()->GetGlobalRotation() * ezVec3(1, 0, 0)).GetNormalized();
  const auto up = (GetOwner()->GetGlobalRotation() * ezVec3(0, 0, 1)).GetNormalized();

  if (pos.IsNaN() || fwd.IsNaN() || up.IsNaN())
    return;

  m_TriggerEntity.SetLocation(AM_Vec3(pos.x, pos.y, pos.z));
  m_TriggerEntity.SetOrientation(AM_Vec3(fwd.x, fwd.y, fwd.z), AM_Vec3(up.x, up.y, up.z));
}

void ezAmplitudeTriggerComponent::UpdateOcclusion()
{
  if (m_uiObstructionOcclusionStateIndex == ezInvalidIndex)
  {
    m_uiObstructionOcclusionStateIndex = static_cast<ezAmplitudeTriggerComponentManager*>(GetOwningManager())->AddObstructionOcclusionState(this);
  }

  if (!m_TriggerEntity.Valid())
    return;

  auto& occlusionState = static_cast<ezAmplitudeTriggerComponentManager*>(GetOwningManager())->GetObstructionOcclusionState(m_uiObstructionOcclusionStateIndex);

  m_TriggerEntity.SetObstruction(occlusionState.m_fLastObstructionValue);
  m_TriggerEntity.SetOcclusion(occlusionState.m_fLastOcclusionValue);
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AmplitudeTriggerComponent);
