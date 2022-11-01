#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/ATL/AudioTranslationLayer.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Profiling/Profiling.h>

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
#  include <AudioSystemPlugin/Core/AudioMiddleware.h>
#  include <RendererCore/Debug/DebugRenderer.h>
#  include <RendererCore/Pipeline/View.h>
#  include <RendererCore/RenderWorld/RenderWorld.h>

ezCVarBool cvar_AudioSystemDebug("Audio.Debugging.Enable", false, ezCVarFlags::None, "Defines if Audio System debug information are displayed.");
#endif

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
  {
    const char* szFile = ":project/Sounds/AudioSystemConfig.ddl";

    ezFileReader file;
    EZ_SUCCEED_OR_RETURN(file.Open(szFile));

    ezOpenDdlReader ddl;
    EZ_SUCCEED_OR_RETURN(ddl.ParseDocument(file));

    const ezOpenDdlReaderElement* pRoot = ddl.GetRootElement();
    const ezOpenDdlReaderElement* pChild = pRoot->GetFirstChild();

    while (pChild)
    {
      if (pChild->IsCustomType("Middleware") && pChild->HasName() && ezStringUtils::Compare(pChild->GetName(), m_pAudioMiddleware->GetMiddlewareName()) == 0)
      {
        ezLog::Debug("Loading audio middleware configuration for {}...", pChild->GetName());

        if (m_pAudioMiddleware->LoadConfiguration(*pChild).Failed())
          ezLog::Error("Failed to load configuration for audio middleware: {0}.", pChild->GetName());

        break;
      }

      pChild = pChild->GetSibling();
    }
  }

  // Start the audio middleware
  const ezResult result = m_pAudioMiddleware->Startup();

  if (result.Succeeded())
  {
    ezLog::Success("ATL loaded successfully. Using {0} as the audio middleware.", m_pAudioMiddleware->GetMiddlewareName());
    return EZ_SUCCESS;
  }

  ezLog::Error("Unable to load the ATL. An error occurred while loading the audio middleware.");
  return EZ_FAILURE;
}

void ezAudioTranslationLayer::Shutdown()
{
  if (m_pAudioMiddleware != nullptr)
    m_pAudioMiddleware->Shutdown().IgnoreResult();

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

ezAudioSystemDataID ezAudioTranslationLayer::GetTriggerId(const char* szTriggerName) const
{
  const auto uiTriggerId = ezHashHelper<const char*>::Hash(szTriggerName);

  if (const auto it = m_mTriggers.Find(uiTriggerId); it.IsValid())
  {
    return uiTriggerId;
  }

  return 0;
}

ezAudioSystemDataID ezAudioTranslationLayer::GetRtpcId(const char* szRtpcName) const
{
  const auto uiRtpcId = ezHashHelper<const char*>::Hash(szRtpcName);

  if (const auto it = m_mRtpcs.Find(uiRtpcId); it.IsValid())
  {
    return uiRtpcId;
  }

  return 0;
}

ezAudioSystemDataID ezAudioTranslationLayer::GetSwitchStateId(const char* szSwitchStateName) const
{
  const auto uiSwitchStateId = ezHashHelper<const char*>::Hash(szSwitchStateName);

  if (const auto it = m_mSwitchStates.Find(uiSwitchStateId); it.IsValid())
  {
    return uiSwitchStateId;
  }

  return 0;
}

ezAudioSystemDataID ezAudioTranslationLayer::GetEnvironmentId(const char* szEnvironmentName) const
{
  const auto uiEnvironmentId = ezHashHelper<const char*>::Hash(szEnvironmentName);

  if (const auto it = m_mEnvironments.Find(uiEnvironmentId); it.IsValid())
  {
    return uiEnvironmentId;
  }

  return 0;
}

void ezAudioTranslationLayer::ProcessRequest(ezVariant&& request)
{
  if (m_pAudioMiddleware == nullptr)
    return;

  bool needCallback = false;

  if (request.IsA<ezAudioSystemRequestRegisterEntity>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestRegisterEntity>();
    ezAudioSystemEntityData* entity = m_pAudioMiddleware->CreateEntityData(audioRequest.m_uiEntityId);

    if (entity == nullptr)
    {
      ezLog::Error("Failed to create entity data for entity {0}", audioRequest.m_uiEntityId);
      return;
    }

    m_mEntities[audioRequest.m_uiEntityId] = EZ_AUDIOSYSTEM_NEW(ezATLEntity, audioRequest.m_uiEntityId, entity);
    audioRequest.m_eStatus = m_pAudioMiddleware->AddEntity(entity, audioRequest.m_sName);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestSetEntityTransform>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetEntityTransform>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set entity transform {0}. Make sure it was registered before.", audioRequest.m_uiEntityId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    audioRequest.m_eStatus = m_pAudioMiddleware->SetEntityTransform(entity->m_pEntityData, audioRequest.m_Transform);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestUnregisterEntity>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnregisterEntity>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to unregister entity {0}. Make sure it was registered before.", audioRequest.m_uiEntityId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    m_pAudioMiddleware->RemoveEntity(entity->m_pEntityData).IgnoreResult();
    audioRequest.m_eStatus = m_pAudioMiddleware->DestroyEntityData(entity->m_pEntityData);

    if (audioRequest.m_eStatus.Succeeded())
    {
      EZ_AUDIOSYSTEM_DELETE(m_mEntities[audioRequest.m_uiEntityId]);
      m_mEntities.Remove(audioRequest.m_uiEntityId);
    }

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestRegisterListener>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestRegisterListener>();
    ezAudioSystemListenerData* pListenerData = m_pAudioMiddleware->CreateListenerData(audioRequest.m_uiListenerId);

    if (pListenerData == nullptr)
    {
      ezLog::Error("Failed to create listener data for listener {0}", audioRequest.m_uiListenerId);
      return;
    }

    m_mListeners[audioRequest.m_uiListenerId] = EZ_AUDIOSYSTEM_NEW(ezATLListener, audioRequest.m_uiListenerId, pListenerData);
    audioRequest.m_eStatus = m_pAudioMiddleware->AddListener(pListenerData, audioRequest.m_sName);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestSetListenerTransform>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetListenerTransform>();

    if (!m_mListeners.Contains(audioRequest.m_uiListenerId))
    {
      ezLog::Error("Failed to set listener transform {0}. Make sure it was registered before.", audioRequest.m_uiListenerId);
      return;
    }

    const auto& listener = m_mListeners[audioRequest.m_uiListenerId];
    audioRequest.m_eStatus = m_pAudioMiddleware->SetListenerTransform(listener->m_pListenerData, audioRequest.m_Transform);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestUnregisterListener>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnregisterListener>();

    if (!m_mListeners.Contains(audioRequest.m_uiListenerId))
    {
      ezLog::Error("Failed to unregister listener {0}. Make sure it was registered before.", audioRequest.m_uiListenerId);
      return;
    }

    const auto& listener = m_mListeners[audioRequest.m_uiListenerId];
    m_pAudioMiddleware->RemoveListener(listener->m_pListenerData).IgnoreResult();
    audioRequest.m_eStatus = m_pAudioMiddleware->DestroyListenerData(listener->m_pListenerData);

    if (audioRequest.m_eStatus.Succeeded())
    {
      EZ_AUDIOSYSTEM_DELETE(m_mListeners[audioRequest.m_uiListenerId]);
      m_mListeners.Remove(audioRequest.m_uiListenerId);
    }

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestLoadTrigger>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestLoadTrigger>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to load trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to load trigger {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return;
    }

    ezAudioSystemEventData* pEventData = m_pAudioMiddleware->CreateEventData(audioRequest.m_uiEventId);

    if (pEventData == nullptr)
    {
      ezLog::Error("Failed to load trigger {0}. Unable to allocate memory for the linked event with ID {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEventId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiObjectId];
    trigger->AttachEvent(audioRequest.m_uiEventId, pEventData);

    audioRequest.m_eStatus = m_pAudioMiddleware->LoadTrigger(entity->m_pEntityData, trigger->m_pTriggerData, pEventData);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestActivateTrigger>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestActivateTrigger>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to activate trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to activate trigger {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiObjectId];

    ezAudioSystemEventData* pEventData = nullptr;
    if (trigger->GetEvent(audioRequest.m_uiEventId, pEventData).Failed())
    {
      ezLog::Error("Failed to activate trigger {0}. Make sure to load the trigger before to activate it.", audioRequest.m_uiObjectId);
      return;
    }

    audioRequest.m_eStatus = m_pAudioMiddleware->ActivateTrigger(entity->m_pEntityData, trigger->m_pTriggerData, pEventData);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestStopEvent>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestStopEvent>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to stop trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiTriggerId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiTriggerId))
    {
      ezLog::Error("Failed to stop trigger {0}. Make sure it was registered before.", audioRequest.m_uiTriggerId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiTriggerId];

    ezAudioSystemEventData* pEventData = nullptr;
    if (trigger->GetEvent(audioRequest.m_uiObjectId, pEventData).Failed())
    {
      ezLog::Error("Failed to stop trigger {0}. Make sure to load the trigger before to stop it.", audioRequest.m_uiObjectId);
      return;
    }

    audioRequest.m_eStatus = m_pAudioMiddleware->StopEvent(entity->m_pEntityData, pEventData);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestUnloadTrigger>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestUnloadTrigger>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to unload the trigger {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mTriggers.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to stop trigger {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& trigger = m_mTriggers[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->UnloadTrigger(entity->m_pEntityData, trigger->m_pTriggerData);

    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestSetRtpcValue>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetRtpcValue>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set the rtpc {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mRtpcs.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to set rtpc {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& rtpc = m_mRtpcs[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetRtpc(entity->m_pEntityData, rtpc->m_pRtpcData, audioRequest.m_fValue);
    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestSetSwitchState>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetSwitchState>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set the switch state {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mSwitchStates.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to set switch state {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& switchState = m_mSwitchStates[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetSwitchState(entity->m_pEntityData, switchState->m_pSwitchStateData);
    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestSetEnvironmentAmount>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestSetEnvironmentAmount>();

    if (!m_mEntities.Contains(audioRequest.m_uiEntityId))
    {
      ezLog::Error("Failed to set environment amount {0}. It references an unregistered entity {1}.", audioRequest.m_uiObjectId, audioRequest.m_uiEntityId);
      return;
    }

    if (!m_mEnvironments.Contains(audioRequest.m_uiObjectId))
    {
      ezLog::Error("Failed to set environment amount {0}. Make sure it was registered before.", audioRequest.m_uiObjectId);
      return;
    }

    const auto& entity = m_mEntities[audioRequest.m_uiEntityId];
    const auto& environment = m_mEnvironments[audioRequest.m_uiObjectId];

    audioRequest.m_eStatus = m_pAudioMiddleware->SetEnvironmentAmount(entity->m_pEntityData, environment->m_pEnvironmentData, audioRequest.m_fAmount);
    needCallback = audioRequest.m_Callback.IsValid();
  }

  else if (request.IsA<ezAudioSystemRequestShutdown>())
  {
    auto& audioRequest = request.GetWritable<ezAudioSystemRequestShutdown>();

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
    needCallback = audioRequest.m_Callback.IsValid();
  }

  if (needCallback)
  {
    ezAudioSystem::GetSingleton()->QueueRequestCallback(std::move(request));
  }
}

void ezAudioTranslationLayer::RegisterTrigger(ezAudioSystemDataID uiId, ezAudioSystemTriggerData* pTriggerData)
{
  if (m_mTriggers.Contains(uiId))
  {
    ezLog::Warning("ATL: Trigger with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_mTriggers[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLTrigger, uiId, pTriggerData);
}

void ezAudioTranslationLayer::RegisterRtpc(ezAudioSystemDataID uiId, ezAudioSystemRtpcData* pRtpcData)
{
  if (m_mRtpcs.Contains(uiId))
  {
    ezLog::Warning("ATL: Rtpc with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_mRtpcs[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLRtpc, uiId, pRtpcData);
}

void ezAudioTranslationLayer::RegisterSwitchState(ezAudioSystemDataID uiId, ezAudioSystemSwitchStateData* pSwitchStateData)
{
  if (m_mSwitchStates.Contains(uiId))
  {
    ezLog::Warning("ATL: Switch state with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_mSwitchStates[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLSwitchState, uiId, pSwitchStateData);
}

void ezAudioTranslationLayer::RegisterEnvironment(ezAudioSystemDataID uiId, ezAudioSystemEnvironmentData* pEnvironmentData)
{
  if (m_mEnvironments.Contains(uiId))
  {
    ezLog::Warning("ATL: Environment with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_mEnvironments[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLEnvironment, uiId, pEnvironmentData);
}

void ezAudioTranslationLayer::UnregisterEntity(const ezAudioSystemDataID uiId)
{
  if (!m_mEntities.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_mEntities[uiId]);
  m_mEntities.Remove(uiId);
}

void ezAudioTranslationLayer::UnregisterListener(const ezAudioSystemDataID uiId)
{
  if (!m_mListeners.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_mListeners[uiId]);
  m_mListeners.Remove(uiId);
}

void ezAudioTranslationLayer::UnregisterTrigger(const ezAudioSystemDataID uiId)
{
  if (!m_mTriggers.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_mTriggers[uiId]);
  m_mTriggers.Remove(uiId);
}

void ezAudioTranslationLayer::UnregisterRtpc(const ezAudioSystemDataID uiId)
{
  if (!m_mRtpcs.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_mRtpcs[uiId]);
  m_mRtpcs.Remove(uiId);
}

void ezAudioTranslationLayer::UnregisterSwitchState(ezAudioSystemDataID uiId)
{
  if (!m_mSwitchStates.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_mSwitchStates[uiId]);
  m_mSwitchStates.Remove(uiId);
}

void ezAudioTranslationLayer::UnregisterEnvironment(ezAudioSystemDataID uiId)
{
  if (!m_mEnvironments.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_mEnvironments[uiId]);
  m_mEnvironments.Remove(uiId);
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

  if (tAccumTime >= ezTime::Seconds(0.5))
  {
    tAccumTime -= ezTime::Seconds(0.5);
    tDisplayedFrameTime = m_LastFrameTime;

    uiFPS = uiFrames * 2;
    uiFrames = 0;
  }

  if (cvar_AudioSystemDebug)
  {
    const auto* pAudioMiddleware = ezSingletonRegistry::GetSingletonInstance<ezAudioMiddleware>();

    if (pAudioMiddleware == nullptr)
      return;

    if (const ezView* pView = ezRenderWorld::GetViewByUsageHint(ezCameraUsageHint::MainView, ezCameraUsageHint::EditorView))
    {
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugRenderer::ScreenPlacement::BottomRight, "AudioSystem", ezFmt("ATL ({0}) - {1} fps, {2} ms", pAudioMiddleware->GetMiddlewareName(), uiFPS, ezArgF(tDisplayedFrameTime.GetMilliseconds(), 1, false, 4)));
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugRenderer::ScreenPlacement::BottomRight, "AudioSystem", ezFmt("Entities Count: {0}", m_mEntities.GetCount()));
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugRenderer::ScreenPlacement::BottomRight, "AudioSystem", ezFmt("Listeners Count: {0}", m_mListeners.GetCount()));
      ezDebugRenderer::DrawInfoText(pView->GetHandle(), ezDebugRenderer::ScreenPlacement::BottomRight, "AudioSystem", ezFmt("Total Allocated Memory: {0}Mb", (ezAudioSystemAllocator::GetSingleton()->GetStats().m_uiAllocationSize + ezAudioMiddlewareAllocator::GetSingleton()->GetStats().m_uiAllocationSize) / 1048576.0f));
    }
  }
}
#endif

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_ATL_AudioTranslationLayer);
