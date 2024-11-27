#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Components/AudioProxyComponent.h>
#include <AudioSystemPlugin/Components/AudioTriggerComponent.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Core/Interfaces/PhysicsWorldModule.h>
#include <Core/Physics/SurfaceResource.h>
#include <Core/ResourceManager/Resource.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Configuration/CVar.h>

constexpr ezTypeVersion kVersion_AudioTriggerComponent = 1;

/// \brief The last used event ID for all audio trigger components.
static ezAudioSystemDataID s_uiNextEventId = 2;

/// \brief A set of generated points distributed in a sphere. This is used
/// for casting rays during the obstruction/occlusion calculation.
static ezVec3 s_InSpherePositions[k_MaxOcclusionRaysCount];

/// \brief Specifies if the s_InSpherePositions array has been initialized.
static bool s_bInSpherePositionsInitialized = false;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
#  include <RendererCore/Debug/DebugRenderer.h>
#  include <RendererCore/Pipeline/View.h>
#  include <RendererCore/RenderWorld/RenderWorld.h>

extern ezCVarBool cvar_AudioSystemDebug;
#endif

/// \brief The number of rays to send each time we should calculate obstruction and occlusion values.
/// \note This value is used only when the trigger's SoundObstructionType property is set to
/// MultipleRay.
ezCVarInt cvar_AudioSystemOcclusionNumRays("Audio.Occlusion.NumRays", 2, ezCVarFlags::Save, "Number of occlusion rays per triggers per frames.");

/// \brief The seed used when generating points in the s_InSpherePositions array.
ezCVarInt cvar_AudioSystemOcclusionRaysSeed("Audio.Occlusion.Seed", 24, ezCVarFlags::Save, "The seed used to generate directions for the trigger's rays.");

/// \brief The maximum distance in world units beyond which the sound Obstruction/Occlusion calculations are disabled.
ezCVarFloat cvar_AudioSystemOcclusionMaxDistance("Audio.Occlusion.MaxDistance", 150.0f, ezCVarFlags::Save, "The maximum distance in world units beyond which the sound obstruction/occlusion calculations are disabled.");

/// \brief The maximum distance after which the Obstruction value starts to decrease with distance.
ezCVarFloat cvar_AudioSystemFullObstructionMaxDistance("Audio.Obstruction.MaxDistance", 5.0f, ezCVarFlags::Save, "The maximum distance after which the obstruction value starts to decrease with distance.");

/// \brief The smooth factor to use when updating the occlusion/obstruction value to the new target over time.
ezCVarFloat cvar_AudioSystemOcclusionSmoothFactor("Audio.Occlusion.SmoothFactor", 5.0f, ezCVarFlags::Save, "How slowly the smoothing of obstruction/occlusion values should smooth to target.");

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAudioTriggerComponent, kVersion_AudioTriggerComponent, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("PlayTrigger", m_sPlayTrigger),
    EZ_MEMBER_PROPERTY("StopTrigger", m_sStopTrigger),
    EZ_ENUM_MEMBER_PROPERTY("SoundObstructionType", ezAudioSystemSoundObstructionType, m_eObstructionType),
    EZ_ACCESSOR_PROPERTY("OcclusionCollisionLayer", GetOcclusionCollisionLayer, SetOcclusionCollisionLayer)->AddAttributes(new ezDynamicEnumAttribute("PhysicsCollisionLayer")),
    EZ_MEMBER_PROPERTY("LoadOnInit", m_bLoadOnInit),
    EZ_MEMBER_PROPERTY("PlayOnActivate", m_bPlayOnActivate),

    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsLoading", IsLoading)->AddAttributes(new ezHiddenAttribute()),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsReady", IsReady)->AddAttributes(new ezHiddenAttribute()),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsStarting", IsStarting)->AddAttributes(new ezHiddenAttribute()),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsPlaying", IsPlaying)->AddAttributes(new ezHiddenAttribute()),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsStopping", IsStopping)->AddAttributes(new ezHiddenAttribute()),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsStopped", IsStopped)->AddAttributes(new ezHiddenAttribute()),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("IsUnloading", IsUnloading)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(Play, In, "Sync"),
    EZ_SCRIPT_FUNCTION_PROPERTY(Stop, In, "Sync"),
    EZ_SCRIPT_FUNCTION_PROPERTY(SetPlayTrigger, In, "PlayTrigger"),
    EZ_SCRIPT_FUNCTION_PROPERTY(SetStopTrigger, In, "StopTrigger"),
    EZ_SCRIPT_FUNCTION_PROPERTY(GetPlayTrigger),
    EZ_SCRIPT_FUNCTION_PROPERTY(GetStopTrigger),
  }
  EZ_END_FUNCTIONS;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

ezAudioTriggerComponentManager::ezAudioTriggerComponentManager(ezWorld* pWorld)
  : ezComponentManager(pWorld)
{
  if (!s_bInSpherePositionsInitialized)
  {
    s_bInSpherePositionsInitialized = true;

    ezRandom rngPhi;
    rngPhi.Initialize(cvar_AudioSystemOcclusionRaysSeed);

    for (auto& pos : s_InSpherePositions)
    {
      pos = ezVec3::MakeRandomPointInSphere(rngPhi);
      pos.SetLength(cvar_AudioSystemOcclusionMaxDistance).IgnoreResult();
    }
  }
}

void ezAudioTriggerComponentManager::Initialize()
{
  SUPER::Initialize();

  {
    auto desc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezAudioTriggerComponentManager::ProcessOcclusion, this);
    desc.m_Phase = ezWorldModule::UpdateFunctionDesc::Phase::Async;
    desc.m_bOnlyUpdateWhenSimulating = true;

    this->RegisterUpdateFunction(desc);
  }

  {
    auto desc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezAudioTriggerComponentManager::Update, this);
    desc.m_Phase = ezWorldModule::UpdateFunctionDesc::Phase::PostTransform;
    desc.m_bOnlyUpdateWhenSimulating = true;

    this->RegisterUpdateFunction(desc);
  }
}

void ezAudioTriggerComponentManager::Deinitialize()
{
  SUPER::Deinitialize();
}

float ezAudioTriggerComponentManager::ObstructionOcclusionValue::GetValue() const
{
  return m_fValue;
}

void ezAudioTriggerComponentManager::ObstructionOcclusionValue::SetTarget(const float fTarget, const bool bReset)
{
  if (bReset)
  {
    Reset(fTarget);
  }
  else if (ezMath::Abs(fTarget - m_fTarget) > ezMath::HugeEpsilon<float>())
  {
    m_fTarget = fTarget;
  }
}

void ezAudioTriggerComponentManager::ObstructionOcclusionValue::Update(const float fSmoothFactor)
{
  if (ezMath::Abs(m_fTarget - m_fValue) > ezMath::HugeEpsilon<float>())
  {
    // Move to the target
    const float smoothFactor = (fSmoothFactor < 0.f) ? cvar_AudioSystemOcclusionSmoothFactor : fSmoothFactor;
    m_fValue += (m_fTarget - m_fValue) / (smoothFactor * smoothFactor + 1.f);
  }
  else
  {
    // Target reached
    m_fValue = m_fTarget;
  }
}

void ezAudioTriggerComponentManager::ObstructionOcclusionValue::Reset(const float fInitialValue)
{
  m_fTarget = m_fValue = fInitialValue;
}

ezUInt32 ezAudioTriggerComponentManager::AddObstructionOcclusionState(ezAudioTriggerComponent* pComponent)
{
  auto& occlusionState = m_ObstructionOcclusionStates.ExpandAndGetRef();
  occlusionState.m_pComponent = pComponent;

  const ezUInt32 uiNumRays = ezMath::Max<int>(cvar_AudioSystemOcclusionNumRays, 1);

  if (const auto* pPhysicsWorldModule = GetWorld()->GetModule<ezPhysicsWorldModuleInterface>())
  {
    if (const auto* listenerManager = GetWorld()->GetComponentManager<ezAudioListenerComponentManager>())
    {
      for (auto it = listenerManager->GetComponents(); it.IsValid(); ++it)
      {
        if (const ezAudioListenerComponent* component = it; component->IsDefault())
        {
          if (const ezVec3 listenerPos = component->GetListenerPosition(); !listenerPos.IsNaN())
          {
            ShootOcclusionRays(occlusionState, listenerPos, uiNumRays, pPhysicsWorldModule);
            break;
          }
        }
      }
    }
  }

  return m_ObstructionOcclusionStates.GetCount() - 1;
}

void ezAudioTriggerComponentManager::RemoveObstructionOcclusionState(ezUInt32 uiIndex)
{
  if (uiIndex >= m_ObstructionOcclusionStates.GetCount())
    return;

  m_ObstructionOcclusionStates.RemoveAtAndSwap(uiIndex);

  if (uiIndex != m_ObstructionOcclusionStates.GetCount())
  {
    m_ObstructionOcclusionStates[uiIndex].m_pComponent->m_uiObstructionOcclusionStateIndex = uiIndex;
  }
}

void ezAudioTriggerComponentManager::ShootOcclusionRays(ObstructionOcclusionState& state, ezVec3 listenerPos, ezUInt32 uiNumRays, const ezPhysicsWorldModuleInterface* pPhysicsWorldModule)
{
  if (state.m_pComponent->m_eObstructionType == ezAudioSystemSoundObstructionType::None)
  {
    state.m_ObstructionValue.Reset();
    state.m_OcclusionValue.Reset();
    return;
  }

  const ezVec3 sourcePos = state.m_pComponent->GetOwner()->GetGlobalPosition();

  // When the source position is invalid for unknown and weird reasons
  if (sourcePos.IsNaN())
    return;

  const ezVec3 directRay = listenerPos - sourcePos;
  const float fDirectRayLength = directRay.GetLength();

  // If the distance between the source and the listener is greater than the maximum allowed distance
  if (fDirectRayLength >= cvar_AudioSystemOcclusionMaxDistance.GetValue())
    return;

  const ezUInt8 uiCollisionLayer = state.m_pComponent->m_uiOcclusionCollisionLayer;

  // Cast direct (obstruction) ray
  CastRay(state, sourcePos, directRay, uiCollisionLayer, pPhysicsWorldModule, 0);
  state.m_ObstructionValue.SetTarget(state.m_ObstructionRaysValues[0]);

  // When multiple rays, compute both obstruction and occlusion
  if (state.m_pComponent->m_eObstructionType == ezAudioSystemSoundObstructionType::MultipleRay)
  {
    float averageOcclusion = 0.0f;

    // Cast indirect (occlusion) rays
    for (ezUInt32 i = 1; i < uiNumRays; ++i)
    {
      const ezUInt32 uiRayIndex = state.m_uiNextRayIndex;

      CastRay(state, sourcePos, s_InSpherePositions[uiRayIndex], uiCollisionLayer, pPhysicsWorldModule, i);
      averageOcclusion += state.m_ObstructionRaysValues[i];

      state.m_uiNextRayIndex = (state.m_uiNextRayIndex + 1) % k_MaxOcclusionRaysCount;
    }

    averageOcclusion /= static_cast<float>(uiNumRays - 1);
    state.m_OcclusionValue.SetTarget(averageOcclusion);

    // Obstruction should be taken into account if the average value of indirect rays is different than the value of the direct ray in the same ray cast,
    // in the other case, the computed obstruction value become the occlusion and obstruction is set to 0
    if (ezMath::Abs(averageOcclusion - state.m_ObstructionRaysValues[0]) <= ezMath::HugeEpsilon<float>())
    {
      state.m_ObstructionValue.Reset();
      state.m_OcclusionValue.SetTarget(state.m_ObstructionRaysValues[0]);
    }
  }
  // Otherwise take the obstruction as the occlusion and set the obstruction to 0
  else
  {
    state.m_ObstructionValue.Reset();
    state.m_OcclusionValue.SetTarget(state.m_ObstructionRaysValues[0]);
  }

  state.m_ObstructionValue.Update();
  state.m_OcclusionValue.Update();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (cvar_AudioSystemDebug)
  {
    if (const ezView* pView = ezRenderWorld::GetViewByUsageHint(ezCameraUsageHint::MainView, ezCameraUsageHint::EditorView))
    {
      ezDebugRenderer::Draw3DText(pView->GetHandle(), ezFmt("Occlusion: {0}\nObstruction: {1}", state.m_OcclusionValue.GetValue(), state.m_ObstructionValue.GetValue()), sourcePos, ezColor::White);
    }
  }
#endif
}

void ezAudioTriggerComponentManager::CastRay(ObstructionOcclusionState& state, ezVec3 sourcePos, ezVec3 direction, ezUInt8 collisionLayer, const ezPhysicsWorldModuleInterface* pPhysicsWorldModule, ezUInt32 rayIndex)
{
  float averageObstruction = 0.0f;

  if (!direction.IsZero())
  {
    const float fDistance = direction.GetLengthAndNormalize();

    ezPhysicsQueryParameters query(collisionLayer);
    query.m_bIgnoreInitialOverlap = true;
    query.m_ShapeTypes = ezPhysicsShapeType::Static | ezPhysicsShapeType::Dynamic | ezPhysicsShapeType::Query;

    ezPhysicsCastResultArray results;

    if (pPhysicsWorldModule->RaycastAll(results, sourcePos, direction, fDistance, query))
    {
      const float fMaxDistance = cvar_AudioSystemOcclusionMaxDistance.GetValue();

      float uiContributedSurfaces = 0;
      for (const auto& hitResult : results.m_Results)
      {
        if (!hitResult.m_hSurface.IsValid())
          continue;

        ezResourceLock hitSurface(hitResult.m_hSurface, ezResourceAcquireMode::PointerOnly);
        if (hitSurface.GetAcquireResult() == ezResourceAcquireResult::MissingFallback)
          continue;

        float obstructionContribution = hitSurface->GetDescriptor().m_fSoundObstruction;

        if (hitResult.m_fDistance > cvar_AudioSystemFullObstructionMaxDistance)
        {
          const float fClampedDistance = ezMath::Clamp(hitResult.m_fDistance, 0.0f, fMaxDistance);
          const float fDistanceScale = 1.0f - (fClampedDistance / fMaxDistance);

          obstructionContribution *= fDistanceScale;
        }

        averageObstruction += obstructionContribution;
        uiContributedSurfaces++;
      }

      averageObstruction /= uiContributedSurfaces;
    }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    if (cvar_AudioSystemDebug)
    {
      if (const ezView* pView = ezRenderWorld::GetViewByUsageHint(ezCameraUsageHint::MainView, ezCameraUsageHint::EditorView))
      {
        ezDebugRenderer::Line ray[1] = {{sourcePos, sourcePos + direction * fDistance}};
        ezDebugRenderer::DrawLines(pView->GetHandle(), ezMakeArrayPtr(ray), averageObstruction == 0.0f ? ezColor::Red : ezColor::Green);
      }
    }
#endif
  }

  if (state.m_ObstructionRaysValues.GetCount() <= rayIndex)
  {
    state.m_ObstructionRaysValues.InsertAt(rayIndex, averageObstruction);
  }
  else
  {
    state.m_ObstructionRaysValues[rayIndex] = averageObstruction;
  }
}

void ezAudioTriggerComponentManager::ProcessOcclusion(const ezWorldModule::UpdateContext& context)
{
  if (const auto pPhysicsWorldModule = GetWorld()->GetModuleReadOnly<ezPhysicsWorldModuleInterface>())
  {
    const ezUInt32 uiNumRays = ezMath::Max<int>(cvar_AudioSystemOcclusionNumRays, 1);

    for (auto& occlusionState : m_ObstructionOcclusionStates)
    {
      if (const auto* audioWorldModule = GetWorld()->GetModuleReadOnly<ezAudioWorldModule>())
      {
        if (const ezAudioListenerComponent* listener = audioWorldModule->GetDefaultListener())
        {
          if (const ezVec3 listenerPos = listener->GetListenerPosition(); !listenerPos.IsNaN())
          {
            ShootOcclusionRays(occlusionState, listenerPos, uiNumRays, pPhysicsWorldModule);
            break;
          }
        }
      }
    }
  }
}

void ezAudioTriggerComponentManager::Update(const ezWorldModule::UpdateContext& context)
{
  for (auto it = this->m_ComponentStorage.GetIterator(context.m_uiFirstComponentIndex, context.m_uiComponentCount); it.IsValid(); ++it)
  {
    if (ComponentType* pComponent = it; pComponent->IsActiveAndInitialized())
    {
      pComponent->Update();
    }
  }
}

ezAudioTriggerComponent::ezAudioTriggerComponent()
  : ezAudioSystemProxyDependentComponent()
  , m_eState(ezAudioSystemTriggerState::Invalid)
  , m_uiPlayEventId(s_uiNextEventId++)
  , m_uiStopEventId(s_uiNextEventId++)
  , m_eObstructionType(ezAudioSystemSoundObstructionType::SingleRay)
  , m_uiOcclusionCollisionLayer(0)
  , m_uiObstructionOcclusionStateIndex(ezInvalidIndex)
  , m_bLoadOnInit(false)
  , m_bPlayOnActivate(false)
{
}

ezAudioTriggerComponent::~ezAudioTriggerComponent() = default;

void ezAudioTriggerComponent::SetOcclusionCollisionLayer(ezUInt8 uiCollisionLayer)
{
  m_uiOcclusionCollisionLayer = uiCollisionLayer;
}

void ezAudioTriggerComponent::SetPlayTrigger(ezString sName)
{
  if (sName == m_sPlayTrigger)
    return;

  if (IsPlaying())
  {
    Stop();
  }

  m_sPlayTrigger = std::move(sName);
}

const ezString& ezAudioTriggerComponent::GetPlayTrigger() const
{
  return m_sPlayTrigger;
}

void ezAudioTriggerComponent::SetStopTrigger(ezString sName)
{
  if (sName == m_sStopTrigger)
    return;

  m_sStopTrigger = std::move(sName);
}

const ezString& ezAudioTriggerComponent::GetStopTrigger() const
{
  return m_sStopTrigger;
}

void ezAudioTriggerComponent::Play(bool bSync)
{
  if (!m_bCanPlay || m_sPlayTrigger.IsEmpty() || IsPlaying() || IsStarting())
    return;

  if (!m_bPlayTriggerLoaded)
    LoadPlayTrigger(true); // Need to be sync if data was not loaded before

  m_eState = ezAudioSystemTriggerState::Starting;

  ezAudioSystemRequestActivateTrigger request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sPlayTrigger);
  request.m_uiEventId = m_uiPlayEventId;

  request.m_Callback = [this](const ezAudioSystemRequestActivateTrigger& e)
  {
    if (e.m_eStatus.Succeeded())
      m_eState = ezAudioSystemTriggerState::Playing;
    else
      m_eState = ezAudioSystemTriggerState::Invalid;
  };

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(request);
  }
}

void ezAudioTriggerComponent::Stop(bool bSync)
{
  StopInternal(bSync, false);
}

const ezEnum<ezAudioSystemTriggerState>& ezAudioTriggerComponent::GetState() const
{
  return m_eState;
}

bool ezAudioTriggerComponent::IsLoading() const
{
  return m_eState == ezAudioSystemTriggerState::Loading;
}

bool ezAudioTriggerComponent::IsReady() const
{
  return m_eState == ezAudioSystemTriggerState::Ready;
}

bool ezAudioTriggerComponent::IsStarting() const
{
  return m_eState == ezAudioSystemTriggerState::Starting;
}

bool ezAudioTriggerComponent::IsPlaying() const
{
  return m_eState == ezAudioSystemTriggerState::Playing;
}

bool ezAudioTriggerComponent::IsStopping() const
{
  return m_eState == ezAudioSystemTriggerState::Stopping;
}

bool ezAudioTriggerComponent::IsStopped() const
{
  return m_eState == ezAudioSystemTriggerState::Stopped;
}

bool ezAudioTriggerComponent::IsUnloading() const
{
  return m_eState == ezAudioSystemTriggerState::Unloading;
}

void ezAudioTriggerComponent::Initialize()
{
  SUPER::Initialize();

  if (m_bLoadOnInit)
  {
    LoadPlayTrigger(false);
    LoadStopTrigger(false, false);
  }
}

void ezAudioTriggerComponent::OnActivated()
{
  SUPER::OnActivated();

  if (m_bCanPlay && m_bPlayOnActivate && !m_bHasPlayedOnActivate)
  {
    Play();
    m_bHasPlayedOnActivate = true;
  }
}

void ezAudioTriggerComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  m_bCanPlay = true;

  if (m_bCanPlay && m_bPlayOnActivate && !m_bHasPlayedOnActivate)
  {
    Play();
    m_bHasPlayedOnActivate = true;
  }
}

void ezAudioTriggerComponent::OnDeactivated()
{
  ezStaticCast<ezAudioTriggerComponentManager*>(GetOwningManager())->RemoveObstructionOcclusionState(m_uiObstructionOcclusionStateIndex);
  m_uiObstructionOcclusionStateIndex = ezInvalidIndex;

  if (IsPlaying())
    StopInternal(false, true);

  m_bHasPlayedOnActivate = false;
  SUPER::OnDeactivated();
}

void ezAudioTriggerComponent::Deinitialize()
{
  if (m_bStopTriggerLoaded)
    UnloadStopTrigger(false, true);

  if (m_bPlayTriggerLoaded)
    UnloadPlayTrigger(false, true);

  SUPER::Deinitialize();
}

void ezAudioTriggerComponent::LoadPlayTrigger(bool bSync)
{
  if (m_sPlayTrigger.IsEmpty())
    return;

  if (m_bPlayTriggerLoaded)
  {
    m_eState = ezAudioSystemTriggerState::Ready;
    return;
  }

  m_eState = ezAudioSystemTriggerState::Loading;

  ezAudioSystemRequestLoadTrigger request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sPlayTrigger);
  request.m_uiEventId = m_uiPlayEventId;

  request.m_Callback = [this](const ezAudioSystemRequestLoadTrigger& m)
  {
    if (m.m_eStatus.Failed())
    {
      m_eState = ezAudioSystemTriggerState::Invalid;
      return;
    }

    m_bPlayTriggerLoaded = true;
    m_eState = ezAudioSystemTriggerState::Ready;
  };

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(request);
  }
}

void ezAudioTriggerComponent::LoadStopTrigger(bool bSync, bool bDeinit)
{
  if (m_sStopTrigger.IsEmpty())
    return;

  if (m_bStopTriggerLoaded)
    return;

  ezAudioSystemRequestLoadTrigger request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sStopTrigger);
  request.m_uiEventId = m_uiStopEventId;

  if (!bDeinit)
  {
    request.m_Callback = [this](const ezAudioSystemRequestLoadTrigger& m)
    {
      if (m.m_eStatus.Failed())
        return;

      m_bStopTriggerLoaded = true;
    };
  }

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(request);
  }

  if (bDeinit)
  {
    m_bStopTriggerLoaded = true;
  }
}

void ezAudioTriggerComponent::UnloadPlayTrigger(bool bSync, bool bDeinit)
{
  if (!m_bPlayTriggerLoaded)
    return;

  m_eState = ezAudioSystemTriggerState::Unloading;

  ezAudioSystemRequestUnloadTrigger request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sPlayTrigger);

  if (!bDeinit)
  {
    request.m_Callback = [this](const ezAudioSystemRequestUnloadTrigger& m)
    {
      if (m.m_eStatus.Failed())
      {
        m_eState = ezAudioSystemTriggerState::Invalid;
        return;
      }

      m_bPlayTriggerLoaded = false;
      m_eState = ezAudioSystemTriggerState::Invalid;
    };
  }

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(request);
  }
}

void ezAudioTriggerComponent::UnloadStopTrigger(bool bSync, bool bDeinit)
{
  if (!m_bStopTriggerLoaded)
    return;

  ezAudioSystemRequestUnloadTrigger request;

  request.m_uiEntityId = GetEntityId();
  request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sStopTrigger);

  if (!bDeinit)
  {
    request.m_Callback = [this](const ezAudioSystemRequestUnloadTrigger& m)
    {
      if (m.m_eStatus.Failed())
        return;

      m_bStopTriggerLoaded = false;
    };
  }

  if (bSync)
  {
    ezAudioSystem::GetSingleton()->SendRequestSync(request);
  }
  else
  {
    ezAudioSystem::GetSingleton()->SendRequest(request);
  }
}

void ezAudioTriggerComponent::UpdateOcclusion()
{
  if (m_pProxyComponent == nullptr)
    return;

  if (m_eObstructionType == ezAudioSystemSoundObstructionType::None)
    return;

  auto* pComponentManager = ezStaticCast<ezAudioTriggerComponentManager*>(GetOwningManager());

  if (m_uiObstructionOcclusionStateIndex == ezInvalidIndex)
    m_uiObstructionOcclusionStateIndex = pComponentManager->AddObstructionOcclusionState(this);

  const auto& occlusionState = pComponentManager->GetObstructionOcclusionState(m_uiObstructionOcclusionStateIndex);

  ezAudioSystemRequestSetObstructionOcclusion request;

  request.m_uiEntityId = m_pProxyComponent->GetEntityId();
  request.m_fObstruction = occlusionState.m_ObstructionValue.GetValue();
  request.m_fOcclusion = occlusionState.m_OcclusionValue.GetValue();

  ezAudioSystem::GetSingleton()->SendRequest(request);
}

void ezAudioTriggerComponent::Update()
{
  if (IsPlaying())
  {
    UpdateOcclusion();
  }
}

void ezAudioTriggerComponent::StopInternal(bool bSync, bool bDeinit)
{
  m_eState = ezAudioSystemTriggerState::Stopping;

  if (m_sStopTrigger.IsEmpty())
  {
    ezAudioSystemRequestStopEvent request;

    request.m_uiEntityId = GetEntityId();
    request.m_uiTriggerId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sPlayTrigger);
    request.m_uiObjectId = m_uiPlayEventId;

    // In case of deinitialization, we don't need to run the callback
    if (!bDeinit)
    {
      request.m_Callback = [this](const ezAudioSystemRequestStopEvent& e)
      {
        if (e.m_eStatus.Succeeded())
          m_eState = ezAudioSystemTriggerState::Stopped;
        else
          m_eState = ezAudioSystemTriggerState::Invalid;
      };
    }

    if (bSync)
    {
      ezAudioSystem::GetSingleton()->SendRequestSync(request);
    }
    else
    {
      ezAudioSystem::GetSingleton()->SendRequest(request);
    }
  }
  else
  {
    if (!m_bStopTriggerLoaded)
      LoadStopTrigger(true, bDeinit); // Need to be sync if data was not loaded before

    ezAudioSystemRequestActivateTrigger request;

    request.m_uiEntityId = GetEntityId();
    request.m_uiObjectId = ezAudioSystem::GetSingleton()->GetTriggerId(m_sStopTrigger);
    request.m_uiEventId = m_uiStopEventId;

    // In case of deinitialization, we don't need to run the callback
    if (!bDeinit)
    {
      request.m_Callback = [this](const ezAudioSystemRequestActivateTrigger& e)
      {
        if (e.m_eStatus.Succeeded())
          m_eState = ezAudioSystemTriggerState::Stopped;
        else
          m_eState = ezAudioSystemTriggerState::Invalid;
      };
    }

    if (bSync)
    {
      ezAudioSystem::GetSingleton()->SendRequestSync(request);
    }
    else
    {
      ezAudioSystem::GetSingleton()->SendRequest(request);
    }
  }

  if (bDeinit)
  {
    m_eState = ezAudioSystemTriggerState::Stopped;
  }
}

void ezAudioTriggerComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AudioTriggerComponent);

  s << m_sPlayTrigger;
  s << m_sStopTrigger;
  s << m_eObstructionType;
  s << m_bLoadOnInit;
  s << m_bPlayOnActivate;
  s << m_uiOcclusionCollisionLayer;
}

void ezAudioTriggerComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AudioTriggerComponent);

  s >> m_sPlayTrigger;
  s >> m_sStopTrigger;
  s >> m_eObstructionType;
  s >> m_bLoadOnInit;
  s >> m_bPlayOnActivate;
  s >> m_uiOcclusionCollisionLayer;
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_Components_AudioTriggerComponent);
