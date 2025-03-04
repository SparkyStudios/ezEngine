#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Core/AudioMiddleware.h>
#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Time/Clock.h>

constexpr ezAudioSystemDataID kEditorListenerId = 1; // EZ will send -1 as ID in override mode, but we use 1 internally
constexpr ezAudioSystemDataID kEditorSoundEventId = 1;

ezThreadID gMainThreadId = static_cast<ezThreadID>(0);

ezCVarFloat cvar_AudioSystemFPS("Audio.FPS", 60, ezCVarFlags::Save, "The maximum number of frames to process within one second in the audio system.");

// clang-format off
EZ_IMPLEMENT_SINGLETON(ezAudioSystem);
// clang-format on

void ezAudioSystem::LoadConfiguration(ezStringView sFile)
{
  ezFileReader file;
  if (file.Open(sFile).Failed())
    return;

  ezOpenDdlReader ddl;
  if (ddl.ParseDocument(file).Failed())
    return;

  const ezOpenDdlReaderElement* pRoot = ddl.GetRootElement();
  const ezOpenDdlReaderElement* pChild = pRoot->GetFirstChild();

  while (pChild)
  {
    if (pChild->IsCustomType("Middleware") && pChild->HasName() && pChild->GetName().Compare(m_AudioTranslationLayer.m_pAudioMiddleware->GetMiddlewareName()) == 0)
    {
      ezLog::Debug("Loading audio middleware configuration for {}...", pChild->GetName());

      if (m_AudioTranslationLayer.m_pAudioMiddleware->LoadConfiguration(*pChild).Failed())
        ezLog::Error("Failed to load configuration for audio middleware: {0}.", pChild->GetName());

      ezLog::Success("Audio middleware configuration for {} successfully loaded.", pChild->GetName());
      break;
    }

    pChild = pChild->GetSibling();
  }
}

void ezAudioSystem::SetOverridePlatform(ezStringView sPlatform)
{
}

void ezAudioSystem::UpdateSound()
{
  // TODO: Seems that this function is not called from the thread that has started the audio system. It is a bit obvious, but have to check later if this assert is relevant...
  // EZ_ASSERT_ALWAYS(gMainThreadId == ezThreadUtils::GetCurrentThreadID(), "AudioSystem::UpdateSound not called from main thread.");

  if (!m_bInitialized)
    return;

  // Process a single synchronous request callback, if any
  bool handleBlockingRequest = false;
  ezVariant blockingRequest;

  {
    EZ_LOCK(m_BlockingRequestCallbacksMutex);
    handleBlockingRequest = !m_BlockingRequestCallbacksQueue.IsEmpty();
    if (handleBlockingRequest)
    {
      blockingRequest = std::move(m_BlockingRequestCallbacksQueue.PeekFront());
      m_BlockingRequestCallbacksQueue.PopFront();
    }
  }

  if (handleBlockingRequest)
  {
    CallRequestCallbackFunc func(blockingRequest);
    ezVariant::DispatchTo(func, blockingRequest.GetType());

    m_ProcessingEvent.ReturnToken();
  }

  if (!handleBlockingRequest)
  {
    // Process asynchronous callbacks
    ezAudioSystemRequestsQueue callbacks{};
    {
      EZ_LOCK(m_PendingRequestCallbacksMutex);
      callbacks.Swap(m_PendingRequestCallbacksQueue);
    }

    while (!callbacks.IsEmpty())
    {
      ezVariant callback(callbacks.PeekFront());

      CallRequestCallbackFunc func(callback);

      ezVariant::DispatchTo(func, callback.GetType());

      callbacks.PopFront();
    }
  }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  m_AudioTranslationLayer.DebugRender();
#endif
}

void ezAudioSystem::SetMasterChannelVolume(float fVolume)
{
  m_AudioTranslationLayer.m_pAudioMiddleware->OnMasterGainChange(fVolume);
}

float ezAudioSystem::GetMasterChannelVolume() const
{
  return m_AudioTranslationLayer.m_pAudioMiddleware->GetMasterGain();
}

void ezAudioSystem::SetMasterChannelMute(bool bMute)
{
  m_AudioTranslationLayer.m_pAudioMiddleware->OnMuteChange(bMute);
}

bool ezAudioSystem::GetMasterChannelMute() const
{
  return m_AudioTranslationLayer.m_pAudioMiddleware->GetMute();
}

void ezAudioSystem::SetMasterChannelPaused(bool bPaused)
{
}

bool ezAudioSystem::GetMasterChannelPaused() const
{
  return false;
}

void ezAudioSystem::SetSoundGroupVolume(ezStringView sVcaGroupGuid, float fVolume)
{
}

float ezAudioSystem::GetSoundGroupVolume(ezStringView sVcaGroupGuid) const
{
  return 0.0f;
}

ezUInt8 ezAudioSystem::GetNumListeners()
{
  return 0;
}

void ezAudioSystem::SetListenerOverrideMode(bool bEnabled)
{
  m_bListenerOverrideMode = bEnabled;
}

void ezAudioSystem::SetListener(ezInt32 iIndex, const ezVec3& vPosition, const ezVec3& vForward, const ezVec3& vUp, const ezVec3& vVelocity)
{
  ezAudioSystemRequestSetListenerTransform request;

  // Index is -1 when inside the editor, listener is overriden when simulating. Both of them seems to mean the listener is the editor camera. Need to be sure about that...
  request.m_uiListenerId = m_bListenerOverrideMode || iIndex == -1 ? kEditorListenerId : static_cast<ezAudioSystemDataID>(iIndex);
  request.m_Transform.m_vPosition = vPosition;
  request.m_Transform.m_vForward = vForward;
  request.m_Transform.m_vUp = vUp;
  request.m_Transform.m_vVelocity = vVelocity;

  if (m_bListenerOverrideMode)
  {
    // Editor mode
    SendRequestSync(request);
  }
  else
  {
    SendRequest(request);
  }
}

ezResult ezAudioSystem::OneShotSound(ezStringView sResourceID, const ezTransform& globalPosition, float fPitch, float fVolume, bool bBlockIfNotLoaded)
{
  ezAudioSystemRequestActivateTrigger request;

  request.m_uiEntityId = 1; // Send the sound to the global entity
  request.m_uiObjectId = GetTriggerId(sResourceID);
  request.m_uiEventId = kEditorSoundEventId;

  ezResult res = EZ_FAILURE;
  request.m_Callback = [&res](const ezAudioSystemRequestActivateTrigger& e)
  {
    res = e.m_eStatus.m_Result;
  };

  SendRequestSync(request);
  return res;
}

ezAudioSystem::ezAudioSystem()
  : m_SingletonRegistrar(this)
  , m_bInitialized(false)
  , m_bListenerOverrideMode(false)
{
  gMainThreadId = ezThreadUtils::GetCurrentThreadID();
}

ezAudioSystem::~ezAudioSystem()
{
  EZ_ASSERT_DEV(!m_bInitialized, "You should shutdown the AudioSystem before to call the dtor.");
}

bool ezAudioSystem::Startup()
{
  EZ_ASSERT_ALWAYS(gMainThreadId == ezThreadUtils::GetCurrentThreadID(), "AudioSystem::Startup not called from main thread.");

  if (m_bInitialized)
    return true;

  const auto* pAudioMiddleware = ezSingletonRegistry::GetSingletonInstance<ezAudioMiddleware>();
  if (pAudioMiddleware == nullptr)
  {
    ezLog::Error("Could not find an active audio middleware. The AudioSystem will not start.");
    return false;
  }

  StopAudioThread();

  if (m_MainEvent.Create().Succeeded() && m_ProcessingEvent.Create().Succeeded() && m_AudioTranslationLayer.Startup().Succeeded())
  {
    // Start audio thread
    StartAudioThread();
    m_bInitialized = true;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    // Register the default (editor) listener
    ezAudioSystemRequestRegisterListener request;

    request.m_uiListenerId = kEditorListenerId;
    request.m_sName = "Editor Listener";

    SendRequestSync(request);
#endif
  }

  return m_bInitialized;
}

void ezAudioSystem::Shutdown()
{
  EZ_ASSERT_ALWAYS(gMainThreadId == ezThreadUtils::GetCurrentThreadID(), "AudioSystem::Shutdown not called from main thread.");

  if (!m_bInitialized)
    return;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  // Unregister the default (editor) listener
  ezAudioSystemRequestUnregisterListener request;

  request.m_uiListenerId = kEditorListenerId;

  SendRequestSync(request);
#endif

  ezAudioSystemRequestShutdown shutdownRequest;
  SendRequestSync(shutdownRequest);

  StopAudioThread();
  m_AudioTranslationLayer.Shutdown();
  m_bInitialized = false;
}

bool ezAudioSystem::IsInitialized() const
{
  return m_bInitialized;
}

void ezAudioSystem::SendRequest(ezVariant&& request)
{
  if (!m_bInitialized)
    return;

  EZ_LOCK(m_PendingRequestsMutex);
  m_PendingRequestsQueue.PushBack(std::move(request));
}

void ezAudioSystem::SendRequests(ezAudioSystemRequestsQueue& requests)
{
  if (!m_bInitialized)
    return;

  EZ_LOCK(m_PendingRequestsMutex);
  for (auto& request : requests)
  {
    m_PendingRequestsQueue.PushBack(std::move(request));
  }
}

void ezAudioSystem::SendRequestSync(ezVariant&& request)
{
  if (!m_bInitialized)
    return;

  {
    EZ_LOCK(m_BlockingRequestsMutex);
    m_BlockingRequestsQueue.PushBack(std::move(request));
  }

  m_ProcessingEvent.ReturnToken();
  m_MainEvent.AcquireToken();
}

void ezAudioSystem::QueueRequestCallback(ezVariant&& request, bool bSync)
{
  if (!m_bInitialized)
    return;

  if (bSync)
  {
    EZ_LOCK(m_BlockingRequestCallbacksMutex);
    m_BlockingRequestCallbacksQueue.PushBack(std::move(request));
  }
  else
  {
    EZ_LOCK(m_PendingRequestCallbacksMutex);
    m_PendingRequestCallbacksQueue.PushBack(std::move(request));
  }
}

ezAudioSystemDataID ezAudioSystem::GetTriggerId(ezStringView sTriggerName) const
{
  return m_AudioTranslationLayer.GetTriggerId(sTriggerName);
}

ezAudioSystemDataID ezAudioSystem::GetRtpcId(ezStringView sRtpcName) const
{
  return m_AudioTranslationLayer.GetRtpcId(sRtpcName);
}

ezAudioSystemDataID ezAudioSystem::GetSwitchStateId(ezStringView sSwitchStateName) const
{
  return m_AudioTranslationLayer.GetSwitchStateId(sSwitchStateName);
}

ezAudioSystemDataID ezAudioSystem::GetEnvironmentId(ezStringView sEnvironmentName) const
{
  return m_AudioTranslationLayer.GetEnvironmentId(sEnvironmentName);
}

ezAudioSystemDataID ezAudioSystem::GetBankId(ezStringView sBankName) const
{
  return 0;
}

void ezAudioSystem::RegisterTrigger(ezAudioSystemDataID uiId, ezAudioSystemTriggerData* pTriggerData)
{
  if (m_AudioTranslationLayer.m_mTriggers.Contains(uiId))
  {
    ezLog::Warning("ATL: Trigger with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_AudioTranslationLayer.m_mTriggers[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLTrigger, uiId, pTriggerData);
}

void ezAudioSystem::RegisterRtpc(ezAudioSystemDataID uiId, ezAudioSystemRtpcData* pRtpcData)
{
  if (m_AudioTranslationLayer.m_mRtpcs.Contains(uiId))
  {
    ezLog::Warning("ATL: Rtpc with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_AudioTranslationLayer.m_mRtpcs[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLRtpc, uiId, pRtpcData);
}

void ezAudioSystem::RegisterSwitchState(ezAudioSystemDataID uiId, ezAudioSystemSwitchStateData* pSwitchStateData)
{
  if (m_AudioTranslationLayer.m_mSwitchStates.Contains(uiId))
  {
    ezLog::Warning("ATL: Switch state with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_AudioTranslationLayer.m_mSwitchStates[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLSwitchState, uiId, pSwitchStateData);
}

void ezAudioSystem::RegisterEnvironment(ezAudioSystemDataID uiId, ezAudioSystemEnvironmentData* pEnvironmentData)
{
  if (m_AudioTranslationLayer.m_mEnvironments.Contains(uiId))
  {
    ezLog::Warning("ATL: Environment with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_AudioTranslationLayer.m_mEnvironments[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLEnvironment, uiId, pEnvironmentData);
}

void ezAudioSystem::RegisterSoundBank(ezAudioSystemDataID uiId, ezAudioSystemBankData* pSoundBankData)
{
  if (m_AudioTranslationLayer.m_mSoundBanks.Contains(uiId))
  {
    ezLog::Warning("ATL: Sound bank with id {0} already exists. Skipping new registration.", uiId);
    return;
  }

  m_AudioTranslationLayer.m_mSoundBanks[uiId] = EZ_AUDIOSYSTEM_NEW(ezATLSoundBank, uiId, pSoundBankData);
}

void ezAudioSystem::UnregisterEntity(const ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mEntities.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mEntities[uiId]);
  m_AudioTranslationLayer.m_mEntities.Remove(uiId);
}

void ezAudioSystem::UnregisterListener(const ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mListeners.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mListeners[uiId]);
  m_AudioTranslationLayer.m_mListeners.Remove(uiId);
}

void ezAudioSystem::UnregisterTrigger(const ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mTriggers.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mTriggers[uiId]);
  m_AudioTranslationLayer.m_mTriggers.Remove(uiId);
}

void ezAudioSystem::UnregisterRtpc(const ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mRtpcs.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mRtpcs[uiId]);
  m_AudioTranslationLayer.m_mRtpcs.Remove(uiId);
}

void ezAudioSystem::UnregisterSwitchState(ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mSwitchStates.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mSwitchStates[uiId]);
  m_AudioTranslationLayer.m_mSwitchStates.Remove(uiId);
}

void ezAudioSystem::UnregisterEnvironment(ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mEnvironments.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mEnvironments[uiId]);
  m_AudioTranslationLayer.m_mEnvironments.Remove(uiId);
}

void ezAudioSystem::UnregisterSoundBank(ezAudioSystemDataID uiId)
{
  if (!m_AudioTranslationLayer.m_mSoundBanks.Contains(uiId))
    return;

  EZ_AUDIOSYSTEM_DELETE(m_AudioTranslationLayer.m_mSoundBanks[uiId]);
  m_AudioTranslationLayer.m_mSoundBanks.Remove(uiId);
}

void ezAudioSystem::UpdateInternal()
{
  EZ_ASSERT_ALWAYS(m_pAudioThread->GetThreadID() == ezThreadUtils::GetCurrentThreadID(), "AudioSystem::UpdateInternal not called from audio thread.");

  EZ_PROFILE_SCOPE("AudioSystem");

  if (!m_bInitialized)
    return;

  const ezTime startTime = ezTime::Now();

  // Process a single synchronous request, if any
  bool handleBlockingRequest = false;
  ezVariant blockingRequest;

  {
    EZ_LOCK(m_BlockingRequestsMutex);
    handleBlockingRequest = !m_BlockingRequestsQueue.IsEmpty();
    if (handleBlockingRequest)
    {
      blockingRequest = std::move(m_BlockingRequestsQueue.PeekFront());
      m_BlockingRequestsQueue.PopFront();
    }
  }

  if (handleBlockingRequest)
  {
    const bool needCallback = m_AudioTranslationLayer.ProcessRequest(std::move(blockingRequest), true);
    m_MainEvent.ReturnToken();

    // If a callback is found, wait for it to be executed
    if (needCallback)
      m_ProcessingEvent.AcquireToken();
  }

  if (!handleBlockingRequest)
  {
    // Normal request processing: lock and swap the pending requests queue
    // so that the queue can be opened for new requests while the current set
    // of requests can be processed.
    ezAudioSystemRequestsQueue requestsToProcess{};
    {
      EZ_LOCK(m_PendingRequestsMutex);
      requestsToProcess.Swap(m_PendingRequestsQueue);
    }

    while (!requestsToProcess.IsEmpty())
    {
      // Normal request...
      ezVariant& request(requestsToProcess.PeekFront());
      m_AudioTranslationLayer.ProcessRequest(std::move(request), false);
      requestsToProcess.PopFront();
    }
  }

  m_AudioTranslationLayer.Update();

  if (!handleBlockingRequest)
  {
    const ezTime endTime = ezTime::Now(); // stamp the end time
    const ezTime elapsedTime = endTime - startTime;

    if (const ezTime frameTime = ezTime::Seconds(1.0f / cvar_AudioSystemFPS); frameTime > elapsedTime)
    {
      const ezTime timeOut = frameTime - elapsedTime;
      EZ_PROFILE_SCOPE_WITH_TIMEOUT("AudioSystem", timeOut);
      m_ProcessingEvent.TryAcquireToken(timeOut).IgnoreResult();
    }
  }
}

void ezAudioSystem::StartAudioThread()
{
  StopAudioThread();

  if (m_pAudioThread == nullptr)
  {
    m_pAudioThread = EZ_AUDIOSYSTEM_NEW(ezAudioThread);
    m_pAudioThread->m_pAudioSystem = this;
    m_pAudioThread->Start();

    ezLog::Success("Audio thread started.");
  }
}

void ezAudioSystem::StopAudioThread()
{
  if (m_pAudioThread != nullptr)
  {
    m_pAudioThread->m_bKeepRunning = false;
    m_pAudioThread->Join();

    EZ_AUDIOSYSTEM_DELETE(m_pAudioThread);

    ezLog::Success("Audio thread stopped.");
  }
}

void ezAudioSystem::GameApplicationEventHandler(const ezGameApplicationExecutionEvent& e)
{
  ezAudioSystem* pAudioSystem = GetSingleton();

  if (pAudioSystem == nullptr || !pAudioSystem->m_bInitialized)
    return;

  if (e.m_Type == ezGameApplicationExecutionEvent::Type::AfterWorldUpdates)
  {
    pAudioSystem->UpdateSound();
  }
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_AudioSystem);
