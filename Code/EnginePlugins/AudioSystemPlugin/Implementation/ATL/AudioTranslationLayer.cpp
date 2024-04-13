#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/ATL/AudioTranslationLayer.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Profiling/Profiling.h>

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
#  include <RendererCore/Debug/DebugRenderer.h>
#  include <RendererCore/Pipeline/View.h>
#  include <RendererCore/RenderWorld/RenderWorld.h>

ezCVarBool cvar_AudioSystemDebug("Audio.Debugging.Enable", false, ezCVarFlags::None, "Defines if Audio System debug information are displayed.");
#endif

ezCVarInt cvar_AudioSystemMemoryEntitiesPoolSize("Audio.Memory.EntitiesPoolSize", 1024, ezCVarFlags::Save, "Specify the pre-allocated number of entities in the pool.");
ezCVarFloat cvar_AudioSystemGain("Audio.MasterGain", 1.0f, ezCVarFlags::Save, "The main volume of the audio system.");
ezCVarBool cvar_AudioSystemMute("Audio.Mute", false, ezCVarFlags::Default, "Whether sound output is muted.");

ezAudioTranslationLayer::ezAudioTranslationLayer() = default;
ezAudioTranslationLayer::~ezAudioTranslationLayer() = default;

ezResult ezAudioTranslationLayer::Startup()
{
  m_pAudioMiddleware = ezSingletonRegistry::GetSingletonInstance<ezAudioMiddleware>();

  if (m_pAudioMiddleware == nullptr)
  {
    ezLog::Error("Unable to load the ATL, there is no audio middleware implementation found. Make sure you have enabled an audio middleware plugin.");
    return EZ_FAILURE;
  }

  // Load configuration
  ezAudioSystem::GetSingleton()->LoadConfiguration(">project/Sounds/AudioSystemConfig.ddl");

  // Start the audio middleware
  EZ_SUCCEED_OR_RETURN_CUSTOM_LOG(m_pAudioMiddleware->Startup(), "Unable to load the ATL. An error occurred while loading the audio middleware.");

  // Register CVar update events
  cvar_AudioSystemGain.m_CVarEvents.AddEventHandler(ezMakeDelegate(&ezAudioTranslationLayer::OnMasterGainChange, this));
  cvar_AudioSystemMute.m_CVarEvents.AddEventHandler(ezMakeDelegate(&ezAudioTranslationLayer::OnMuteChange, this));

  ezLog::Success("ATL loaded successfully. Using {0} as the audio middleware.", m_pAudioMiddleware->GetMiddlewareName());
  return EZ_SUCCESS;
}

void ezAudioTranslationLayer::Shutdown()
{
  if (m_pAudioMiddleware != nullptr)
  {
    // Unregister CVar update events
    cvar_AudioSystemGain.m_CVarEvents.RemoveEventHandler(ezMakeDelegate(&ezAudioTranslationLayer::OnMasterGainChange, this));
    cvar_AudioSystemMute.m_CVarEvents.RemoveEventHandler(ezMakeDelegate(&ezAudioTranslationLayer::OnMuteChange, this));

    m_pAudioMiddleware->Shutdown().IgnoreResult();
  }

  m_pAudioMiddleware = nullptr;

  m_mEntities.Clear();
  m_mListeners.Clear();
  m_mTriggers.Clear();
  m_mRtpcs.Clear();
  m_mSwitchStates.Clear();
}

void ezAudioTranslationLayer::Update()
{
  EZ_PROFILE_SCOPE("AudioSystem");

  const ezTime currentUpdateTime = ezTime::Now();
  m_LastFrameTime = currentUpdateTime - m_LastUpdateTime;
  m_LastUpdateTime = currentUpdateTime;

  auto* pAudioMiddleware = ezSingletonRegistry::GetSingletonInstance<ezAudioMiddleware>();

  if (pAudioMiddleware == nullptr)
    return;

  pAudioMiddleware->Update(m_LastFrameTime);
}

ezAudioSystemDataID ezAudioTranslationLayer::GetTriggerId(ezStringView sTriggerName) const
{
  const auto uiTriggerId = ezHashHelper<ezStringView>::Hash(sTriggerName);

  if (const auto it = m_mTriggers.Find(uiTriggerId); it.IsValid())
  {
    return uiTriggerId;
  }

  return 0;
}

ezAudioSystemDataID ezAudioTranslationLayer::GetRtpcId(ezStringView sRtpcName) const
{
  const auto uiRtpcId = ezHashHelper<ezStringView>::Hash(sRtpcName);

  if (const auto it = m_mRtpcs.Find(uiRtpcId); it.IsValid())
  {
    return uiRtpcId;
  }

  return 0;
}

ezAudioSystemDataID ezAudioTranslationLayer::GetSwitchStateId(ezStringView sSwitchStateName) const
{
  const auto uiSwitchStateId = ezHashHelper<ezStringView>::Hash(sSwitchStateName);

  if (const auto it = m_mSwitchStates.Find(uiSwitchStateId); it.IsValid())
  {
    return uiSwitchStateId;
  }

  return 0;
}

ezAudioSystemDataID ezAudioTranslationLayer::GetEnvironmentId(ezStringView sEnvironmentName) const
{
  const auto uiEnvironmentId = ezHashHelper<ezStringView>::Hash(sEnvironmentName);

  if (const auto it = m_mEnvironments.Find(uiEnvironmentId); it.IsValid())
  {
    return uiEnvironmentId;
  }

  return 0;
}

bool ezAudioTranslationLayer::ProcessRequest(ezVariant&& request, bool bSync)
{
  if (m_pAudioMiddleware == nullptr)
    return false;

  bool needCallback = false;

  if (request.IsA<ezAudioSystemRequestRegisterEntity>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestRegisterEntity>();
    ezAudioSystemEntityData* entity = m_pAudioMiddleware->CreateEntityData(audioRequest.m_uiEntityId);

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (entity == nullptr)
    {
      ezLog::Error("Failed to create entity data for entity {0}", audioRequest.m_uiEntityId);
      return needCallback;
    }

    m_mEntities[audioRequest.m_uiEntityId] = EZ_AUDIOSYSTEM_NEW(ezATLEntity, audioRequest.m_uiEntityId, entity);
    audioRequest.m_eStatus = m_pAudioMiddleware->AddEntity(entity, audioRequest.m_sName);
  }

  else if (request.IsA<ezAudioSystemRequestSetEntityTransform>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetEntityTransform>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set entity transform {0}. Make sure it was registered before.", audioRequest.m_uiEntityId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    audioRequest.m_eStatus = m_pAudioMiddleware->SetEntityTransform(entity->m_pEntityData, audioRequest.m_Transform);
  }

  else if (request.IsA<ezAudioSystemRequestUnregisterEntity>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnregisterEntity>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to unregister entity {0}. Make sure it was registered before.", audioRequest.m_uiEntityId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    m_pAudioMiddleware->RemoveEntity(entity->m_pEntityData).IgnoreResult();
    audioRequest.m_eStatus = m_pAudioMiddleware->DestroyEntityData(entity->m_pEntityData);

    if (audioRequest.m_eStatus.Succeeded())
    {
      EZ_AUDIOSYSTEM_DELETE(m_mEntities[audioRequest.m_uiEntityId]);
      m_mEntities.Remove(audioRequest.m_uiEntityId);
    }
  }

  else if (request.IsA<ezAudioSystemRequestRegisterListener>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestRegisterListener>();
    ezAudioSystemListenerData* pListenerData = m_pAudioMiddleware->CreateListenerData(audioRequest.m_uiListenerId);

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (pListenerData == nullptr)
    {
      ezLog::Error("Failed to create listener data for listener {0}", audioRequest.m_uiListenerId);
      return needCallback;
    }

    m_mListeners[audioRequest.m_uiListenerId] = EZ_AUDIOSYSTEM_NEW(ezATLListener, audioRequest.m_uiListenerId, pListenerData);
    audioRequest.m_eStatus = m_pAudioMiddleware->AddListener(pListenerData, audioRequest.m_sName);
  }

  else if (request.IsA<ezAudioSystemRequestSetListenerTransform>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetListenerTransform>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mListeners.Contains(audioRequest.m_uiListenerId))
    {
      ezLog::Error("Failed to set listener transform {0}. Make sure it was registered before.", audioRequest.m_uiListenerId);
      return needCallback;
    }

    const auto& listener = m_mListeners[audioRequest.m_uiListenerId];
    audioRequest.m_eStatus = m_pAudioMiddleware->SetListenerTransform(listener->m_pListenerData, audioRequest.m_Transform);
  }

  else if (request.IsA<ezAudioSystemRequestUnregisterListener>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnregisterListener>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mListeners.Contains(audioRequest.m_uiListenerId))
    {
      ezLog::Error("Failed to unregister listener {0}. Make sure it was registered before.", audioRequest.m_uiListenerId);
      return needCallback;
    }

    const auto& listener = m_mListeners[audioRequest.m_uiListenerId];
    m_pAudioMiddleware->RemoveListener(listener->m_pListenerData).IgnoreResult();
    audioRequest.m_eStatus = m_pAudioMiddleware->DestroyListenerData(listener->m_pListenerData);

    if (audioRequest.m_eStatus.Succeeded())
    {
      EZ_AUDIOSYSTEM_DELETE(m_mListeners[audioRequest.m_uiListenerId]);
      m_mListeners.Remove(audioRequest.m_uiListenerId);
    }
  }

  else if (request.IsA<ezAudioSystemRequestLoadTrigger>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestLoadTrigger>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to load trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to load trigger {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiObjectId];

    ezAudioSystemEventData* pEventData = nullptr;
    if (trigger->GetEvent(audioRequest.m_uiEventId, pEventData).Failed())
    {
      pEventData = m_pAudioMiddleware->CreateEventData(audioRequest.m_uiEventId);

      if (pEventData == nullptr)
      {
        ezLog::Error("Failed to load trigger {0}. Unable to allocate memory for the linked event with ID {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEventId);
        return needCallback;
      }

      trigger->AttachEvent(audioRequest.m_uiEventId, pEventData);
    }

    audioRequest.m_eStatus = m_pAudioMiddleware->LoadTrigger(entity->m_pEntityData, trigger->m_pTriggerData, pEventData);
  }

  else if (request.IsA<ezAudioSystemRequestActivateTrigger>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestActivateTrigger>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to activate trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to activate trigger {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiObjectId];

    ezAudioSystemEventData* pEventData = nullptr;
    if (trigger->GetEvent(audioRequest.m_uiEventId, pEventData).Failed())
    {
      ezLog::Error("Failed to activate trigger {0}. Make sure to load the trigger before to activate it.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    audioRequest.m_eStatus = m_pAudioMiddleware->ActivateTrigger(entity->m_pEntityData, trigger->m_pTriggerData, pEventData);
  }

  else if (request.IsA<ezAudioSystemRequestStopEvent>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestStopEvent>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to stop trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiTriggerId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiTriggerId))
    {
      ezLog::Error("Failed to stop trigger {0}. Make sure it was registered before.", audioRequest.m_uiTriggerId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiTriggerId];

    ezAudioSystemEventData* pEventData = nullptr;
    if (trigger->GetEvent(audioRequest.m_uiObjectId, pEventData).Failed())
    {
      ezLog::Error("Failed to stop trigger {0}. Make sure to load the trigger before to stop it.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    audioRequest.m_eStatus = m_pAudioMiddleware->StopEvent(entity->m_pEntityData, pEventData);
  }

  else if (request.IsA<ezAudioSystemRequestUnloadTrigger>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnloadTrigger>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to unload the trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to stop trigger {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->UnloadTrigger(entity->m_pEntityData, trigger->m_pTriggerData);
  }

  else if (request.IsA<ezAudioSystemRequestSetRtpcValue>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetRtpcValue>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set the rtpc {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mRtpcs.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to set rtpc {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& rtpc = m_mRtpcs[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetRtpc(entity->m_pEntityData, rtpc->m_pRtpcData, audioRequest.m_fValue);
  }

  else if (request.IsA<ezAudioSystemRequestSetSwitchState>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetSwitchState>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set the switch state {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mSwitchStates.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to set switch state {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& switchState = m_mSwitchStates[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetSwitchState(entity->m_pEntityData, switchState->m_pSwitchStateData);
  }

  else if (request.IsA<ezAudioSystemRequestSetEnvironmentAmount>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetEnvironmentAmount>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set amount for environment {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return needCallback;
    }

    if (!m_mEnvironments.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to set amount for environment {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& environment = m_mEnvironments[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetEnvironmentAmount(entity->m_pEntityData, environment->m_pEnvironmentData, audioRequest.m_fAmount);
  }

  else if (request.IsA<ezAudioSystemRequestSetObstructionOcclusion>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetObstructionOcclusion>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set obstruction and occlusion values. It references an unregistered entity {0}.", audioRequest.m_uiEntityId);
      return needCallback;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetObstructionAndOcclusion(entity->m_pEntityData, audioRequest.m_fObstruction, audioRequest.m_fOcclusion);
  }

  else if (request.IsA<ezAudioSystemRequestLoadBank>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestLoadBank>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mSoundBanks.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to load sound bank {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& bank = m_mSoundBanks[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->LoadBank(bank->m_pSoundBankData);
  }

  else if (request.IsA<ezAudioSystemRequestUnloadBank>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnloadBank>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    if (!m_mSoundBanks.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to unload sound bank {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return needCallback;
    }

    const auto& bank = m_mSoundBanks[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->UnloadBank(bank->m_pSoundBankData);
  }

  else if (request.IsA<ezAudioSystemRequestShutdown>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestShutdown>();

    audioRequest.m_eStatus = {EZ_FAILURE};
    needCallback = audioRequest.m_Callback.IsValid();

    // Destroy sound banks
    for (auto&& bank : m_mSoundBanks)
    {
      m_pAudioMiddleware->DestroyBank(bank.Value()->m_pSoundBankData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(bank.Value());
    }

    // Destroy environments
    for (auto&& environment : m_mEnvironments)
    {
      m_pAudioMiddleware->DestroyEnvironmentData(environment.Value()->m_pEnvironmentData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(environment.Value());
    }

    // Destroy switch states
    for (auto&& switchState : m_mSwitchStates)
    {
      m_pAudioMiddleware->DestroySwitchStateData(switchState.Value()->m_pSwitchStateData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(switchState.Value());
    }

    // Destroy rtpcs
    for (auto&& rtpc : m_mRtpcs)
    {
      m_pAudioMiddleware->DestroyRtpcData(rtpc.Value()->m_pRtpcData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(rtpc.Value());
    }

    // Destroy triggers
    for (auto&& trigger : m_mTriggers)
    {
      m_pAudioMiddleware->DestroyTriggerData(trigger.Value()->m_pTriggerData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(trigger.Value());
    }

    // Destroy listeners
    for (auto&& listener : m_mListeners)
    {
      m_pAudioMiddleware->DestroyListenerData(listener.Value()->m_pListenerData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(listener.Value());
    }

    // Destroy entities
    for (auto&& entity : m_mEntities)
    {
      m_pAudioMiddleware->DestroyEntityData(entity.Value()->m_pEntityData).IgnoreResult();
      EZ_AUDIOSYSTEM_DELETE(entity.Value());
    }

    audioRequest.m_eStatus = {EZ_SUCCESS};
  }

  if (needCallback)
  {
    ezAudioSystem::GetSingleton()->QueueRequestCallback(std::move(request), bSync);
  }

  return needCallback;
}

void ezAudioTranslationLayer::OnMasterGainChange(const ezCVarEvent& e) const
{
  if (e.m_EventType == ezCVarEvent::Type::ValueChanged && m_pAudioMiddleware != nullptr)
  {
    m_pAudioMiddleware->OnMasterGainChange(static_cast<ezCVarFloat*>(e.m_pCVar)->GetValue());
  }
}

void ezAudioTranslationLayer::OnMuteChange(const ezCVarEvent& e) const
{
  if (e.m_EventType == ezCVarEvent::Type::ValueChanged && m_pAudioMiddleware != nullptr)
  {
    m_pAudioMiddleware->OnMuteChange(static_cast<ezCVarBool*>(e.m_pCVar)->GetValue());
  }
}

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
void ezAudioTranslationLayer::DebugRender() const
{
  static ezTime tAccumTime;
  static ezTime tDisplayedFrameTime = m_LastFrameTime;
  static ezUInt32 uiFrames = 0;
  static ezUInt32 uiFPS = 0;

  ++uiFrames;
  tAccumTime += m_LastFrameTime;

  if (tAccumTime >= ezTime::Seconds(1.0))
  {
    tAccumTime -= ezTime::Seconds(1.0);
    tDisplayedFrameTime = m_LastFrameTime;

    uiFPS = uiFrames;
    uiFrames = 0;
  }

  if (cvar_AudioSystemDebug)
  {
    const auto* pAudioMiddleware = ezSingletonRegistry::GetSingletonInstance<ezAudioMiddleware>();

    if (pAudioMiddleware == nullptr)
      return;

    if (const ezView* pView = ezRenderWorld::GetViewByUsageHint(ezCameraUsageHint::MainView, ezCameraUsageHint::EditorView))
    {
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugTextPlacement::BottomRight, "AudioSystem", ezFmt("ATL ({0}) - {1} fps, {2} ms", pAudioMiddleware->GetMiddlewareName(), uiFPS, ezArgF(tDisplayedFrameTime.GetMilliseconds(), 1, false, 4)));
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugTextPlacement::BottomRight, "AudioSystem", ezFmt("Entities Count: {0}", m_mEntities.GetCount()));
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugTextPlacement::BottomRight, "AudioSystem", ezFmt("Listeners Count: {0}", m_mListeners.GetCount()));
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugTextPlacement::BottomRight, "AudioSystem", ezFmt("Total Allocated Memory: {0}Mb", (ezAudioSystemAllocator::GetSingleton()->GetStats().m_uiAllocationSize + ezAudioMiddlewareAllocator::GetSingleton()->GetStats().m_uiAllocationSize) / 1048576.0f));
    }
  }
}
#endif

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_ATL_AudioTranslationLayer);
