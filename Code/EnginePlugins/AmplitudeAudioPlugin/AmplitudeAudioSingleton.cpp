#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>
#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>
#include <AmplitudeAudioPlugin/Core/Common.h>

#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <Core/ResourceManager/ResourceManager.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>

#include <GameEngine/GameApplication/GameApplication.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common/Constants.h>

using namespace SparkyStudios::Audio;

namespace Log
{
  static void Write(const char* format, va_list args)
  {
    if (format && format[0] != '\0')
    {
      constexpr size_t bufferLen = 1024;
      char buffer[bufferLen] = "[Amplitude] ";

      vsnprintf(buffer + 12, bufferLen - 12, format, args);

      buffer[bufferLen - 1] = '\0';

      ezLog::Debug("{0}", buffer);
    }
  }

  static void DeviceNotification(Amplitude::DeviceNotification notification, const Amplitude::DeviceDescription& device, Amplitude::Driver* driver)
  {
    switch (notification)
    {
      case Amplitude::DeviceNotification::Opened:
        ezLog::Info("Device opened: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Started:
        ezLog::Info("Device started: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Stopped:
        ezLog::Info("Device stopped: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Rerouted:
        ezLog::Info("Device rerouted: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Closed:
        ezLog::Info("Device closed: {0}", device.mDeviceName.c_str());
        break;
    }
  }
} // namespace Log

namespace Memory
{
  static Amplitude::AmVoidPtr Malloc([[maybe_unused]] Amplitude::MemoryPoolKind pool, Amplitude::AmSize size)
  {
    return ezAudioMiddlewareAllocatorWrapper::GetAllocator()->Allocate(size, EZ_AUDIOSYSTEM_MEMORY_ALIGNMENT);
  }

  static Amplitude::AmVoidPtr Malign([[maybe_unused]] Amplitude::MemoryPoolKind pool, Amplitude::AmSize size, Amplitude::AmUInt32 alignment)
  {
    return ezAudioMiddlewareAllocatorWrapper::GetAllocator()->Allocate(size, alignment);
  }

  static Amplitude::AmVoidPtr Realloc(Amplitude::MemoryPoolKind pool, Amplitude::AmVoidPtr address, Amplitude::AmSize size)
  {
    if (address == nullptr)
      return Malloc(pool, size);

    return ezAudioMiddlewareAllocatorWrapper::GetAllocator()->Reallocate(address, ezAudioMiddlewareAllocatorWrapper::GetAllocator()->AllocatedSize(address), size, EZ_AUDIOSYSTEM_MEMORY_ALIGNMENT);
  }

  static Amplitude::AmVoidPtr Realign(Amplitude::MemoryPoolKind pool, Amplitude::AmVoidPtr address, Amplitude::AmSize size, Amplitude::AmUInt32 alignment)
  {
    if (address == nullptr)
      return Malign(pool, size, alignment);

    return ezAudioMiddlewareAllocatorWrapper::GetAllocator()->Reallocate(address, ezAudioMiddlewareAllocatorWrapper::GetAllocator()->AllocatedSize(address), size, alignment);
  }

  static void Free([[maybe_unused]] Amplitude::MemoryPoolKind pool, Amplitude::AmVoidPtr address)
  {
    ezAudioMiddlewareAllocatorWrapper::GetAllocator()->Deallocate(address);
  }

  static Amplitude::AmSize TotalMemorySize()
  {
    return ezAudioMiddlewareAllocatorWrapper::GetAllocator()->GetStats().m_uiAllocationSize;
  }

  static Amplitude::AmSize SizeOfMemory([[maybe_unused]] Amplitude::MemoryPoolKind pool, Amplitude::AmConstVoidPtr address)
  {
    return ezAudioMiddlewareAllocatorWrapper::GetAllocator()->AllocatedSize(address);
  }
} // namespace Memory

namespace Utils
{
  static AmVec3 ezVec3ToAmVec3(const ezVec3& vec)
  {
    return AM_V3(vec.x, vec.y, vec.z);
  }
} // namespace Utils

EZ_IMPLEMENT_SINGLETON(ezAmplitude);

void ezAmplitudeConfiguration::Save(ezOpenDdlWriter& ddl) const
{
  ezOpenDdlUtils::StoreString(ddl, m_sInitSoundBank, s_szAmplitudeConfigKeyInitBank);
  ezOpenDdlUtils::StoreString(ddl, m_sEngineConfigFileName, s_szAmplitudeConfigKeyEngineConfigFileName);
}

void ezAmplitudeConfiguration::Load(const ezOpenDdlReaderElement& ddl)
{
  if (const ezOpenDdlReaderElement* pElement = ddl.FindChildOfType(ezOpenDdlPrimitiveType::String, s_szAmplitudeConfigKeyInitBank))
  {
    m_sInitSoundBank = pElement->GetPrimitivesString()[0];
  }

  if (const ezOpenDdlReaderElement* pElement = ddl.FindChildOfType(ezOpenDdlPrimitiveType::String, s_szAmplitudeConfigKeyEngineConfigFileName))
  {
    m_sEngineConfigFileName = pElement->GetPrimitivesString()[0];
  }
}

bool ezAmplitudeConfiguration::operator==(const ezAmplitudeConfiguration& rhs) const
{
  if (m_sInitSoundBank != rhs.m_sInitSoundBank)
    return false;

  if (m_sEngineConfigFileName != rhs.m_sEngineConfigFileName)
    return false;

  return true;
}

ezResult ezAmplitudeAssetProfiles::Save(ezOpenDdlWriter& writer) const
{
  if (m_AssetProfiles.IsEmpty())
    return EZ_FAILURE;

  for (auto it = m_AssetProfiles.GetIterator(); it.IsValid(); ++it)
  {
    if (!it.Key().IsEmpty())
    {
      writer.BeginObject("Platform", it.Key());
      {
        it.Value().Save(writer);
      }
      writer.EndObject();
    }
  }

  return EZ_SUCCESS;
}

ezResult ezAmplitudeAssetProfiles::Load(const ezOpenDdlReaderElement& reader)
{
  m_AssetProfiles.Clear();

  const ezOpenDdlReaderElement* pChild = reader.GetFirstChild();

  while (pChild)
  {
    if (pChild->IsCustomType("Platform") && pChild->HasName())
    {
      auto& cfg = m_AssetProfiles[pChild->GetName()];

      cfg.Load(*pChild);
    }

    pChild = pChild->GetSibling();
  }

  return EZ_SUCCESS;
}


ezAmplitude::ezAmplitude()
  : m_SingletonRegistrar(this)
  , m_pEngine(nullptr)
  , m_dCurrentTime(0.0)
  , m_bInitialized(false)
{
  m_pData = EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), Data);
}

ezAmplitude::~ezAmplitude()
{
  // Shutdown().IgnoreResult();
}

ezResult ezAmplitude::LoadConfiguration(const ezOpenDdlReaderElement& reader)
{
  return m_pData->m_Configs.Load(reader);
}

ezResult ezAmplitude::Startup()
{
  if (m_bInitialized)
    return EZ_SUCCESS;

  Amplitude::MemoryManagerConfig memConfig;
  memConfig.alignedMalloc = Memory::Malign;
  memConfig.alignedRealloc = Memory::Realign;
  memConfig.free = Memory::Free;
  memConfig.malloc = Memory::Malloc;
  memConfig.realloc = Memory::Realloc;
  memConfig.sizeOf = Memory::SizeOfMemory;
  memConfig.totalReservedMemorySize = Memory::TotalMemorySize;

  Amplitude::MemoryManager::Initialize(memConfig);
  EZ_ASSERT_DEBUG(Amplitude::MemoryManager::IsInitialized(), "Amplitude memory manager not initialized.");

  DetectPlatform();

  if (!m_pData->m_Configs.m_AssetProfiles.Contains(m_pData->m_sPlatform))
  {
    ezLog::Error("Amplitude configuration for platform '{0}' not available. Amplitude will be deactivated.", m_pData->m_sPlatform);
    return EZ_FAILURE;
  }

  const auto& config = m_pData->m_Configs.m_AssetProfiles[m_pData->m_sPlatform];

  // Initialize the engine
  Amplitude::RegisterLogFunc(Log::Write);
  Amplitude::RegisterDeviceNotificationCallback(Log::DeviceNotification);

  ezStringBuilder assetsPath;

  if (ezFileSystem::ResolvePath(":project/Sounds/Amplitude/" AMPLITUDE_ASSETS_DIR_NAME, &assetsPath, nullptr).Failed())
  {
    ezLog::Error("No Amplitude assets directory available in the project. Amplitude will be deactivated.", m_pData->m_sPlatform);
    return EZ_FAILURE;
  }

  Amplitude::Engine::RegisterDefaultPlugins();

  m_pEngine = Amplitude::Engine::GetInstance();
  EZ_ASSERT_DEBUG(m_pEngine != nullptr, "Amplitude engine not available.");

  m_Loader.SetBasePath(AM_STRING_TO_OS_STRING(assetsPath.GetData()));
  m_pEngine->SetFileSystem(&m_Loader);

  // Wait for the file system to complete loading
  m_pEngine->StartOpenFileSystem();
  while (!m_pEngine->TryFinalizeOpenFileSystem())
    Amplitude::Thread::Sleep(1);

  const auto sPluginsPath = m_Loader.ResolvePath(AM_OS_STRING("plugins"));
  Amplitude::Engine::AddPluginSearchPath(sPluginsPath);

  // Auto load available plugins
  {
    ezFileSystemIterator fsIt;
    for (fsIt.StartSearch(AM_OS_STRING_TO_STRING(sPluginsPath), ezFileSystemIteratorFlags::ReportFiles); fsIt.IsValid(); fsIt.Next())
    {
      ezStringBuilder fileName = fsIt.GetStats().m_sName;
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      if (fileName.EndsWith_NoCase("_d.dll"))
#else
      if (fileName.EndsWith_NoCase(".dll"))
#endif
      {
        ezStringBuilder sTemp;
        Amplitude::Engine::LoadPlugin(AM_STRING_TO_OS_STRING(fileName.GetFileName().GetData(sTemp)));
      }
    }
  }

  if (!m_pEngine->Initialize(AM_STRING_TO_OS_STRING(config.m_sEngineConfigFileName.GetData())))
  {
    ezLog::Error("Amplitude engine initialization failed with the config file '{0}'.", config.m_sEngineConfigFileName);
    return EZ_FAILURE;
  }

  Amplitude::AmBankID uiInitBankId = Amplitude::kAmInvalidObjectId;

  if (!m_pEngine->LoadSoundBank(AM_STRING_TO_OS_STRING(config.m_sInitSoundBank.GetData()), uiInitBankId))
  {
    ezLog::Error("Amplitude engine initialization failed. Could not load initial sound bank '{0}'.", config.m_sInitSoundBank);
    return EZ_FAILURE;
  }

  // Wait for the sound files to complete loading
  m_pEngine->StartLoadSoundFiles();
  while (!m_pEngine->TryFinalizeLoadSoundFiles())
    Amplitude::Thread::Sleep(1);

  m_bInitialized = true;

  ezLog::Success("Amplitude Audio initialized.");
  return EZ_SUCCESS;
}

ezResult ezAmplitude::Shutdown()
{
  if (m_bInitialized)
  {
    m_bInitialized = false;

    if (m_pEngine != nullptr)
    {
      m_pEngine->Deinitialize();

      // Wait for the file system to complete closing
      m_pEngine->StartCloseFileSystem();
      while (!m_pEngine->TryFinalizeCloseFileSystem())
        Amplitude::Thread::Sleep(1);

      Amplitude::Engine::DestroyInstance();
      m_pEngine = nullptr;
    }

    Amplitude::Engine::UnregisterDefaultPlugins();

    // Terminate the Memory Manager
    if (Amplitude::MemoryManager::IsInitialized())
    {
      Amplitude::MemoryManager::Deinitialize();
    }

    // Finally delete all data
    m_pData.Clear();

    ezLog::Success("Amplitude Audio deinitialized.");
  }

  return EZ_SUCCESS;
}

ezResult ezAmplitude::Release()
{
  return EZ_SUCCESS;
}

ezResult ezAmplitude::StopAllSounds()
{
  if (m_bInitialized)
  {
    m_pEngine->StopAll();
  }

  return EZ_SUCCESS;
}

ezResult ezAmplitude::AddEntity(ezAudioSystemEntityData* pEntityData, const char* szEntityName)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const Amplitude::Entity& entity = m_pEngine->AddEntity(pAmplitudeEntity->m_uiAmId);

  return entity.Valid() ? EZ_SUCCESS : EZ_FAILURE;
}

ezResult ezAmplitude::ResetEntity(ezAudioSystemEntityData* pEntityData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  // Nothing to reset for now...

  return EZ_SUCCESS;
}

ezResult ezAmplitude::UpdateEntity(ezAudioSystemEntityData* pEntityData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  // Nothing to update for now... The real entity update is scheduled in the Amplitude's Update function.

  return EZ_SUCCESS;
}

ezResult ezAmplitude::RemoveEntity(ezAudioSystemEntityData* pEntityData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  m_pEngine->RemoveEntity(pAmplitudeEntity->m_uiAmId);
  const Amplitude::Entity& entity = m_pEngine->GetEntity(pAmplitudeEntity->m_uiAmId);

  return entity.Valid() ? EZ_FAILURE : EZ_SUCCESS;
}

ezResult ezAmplitude::SetEntityTransform(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTransform& Transform)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  if (const Amplitude::Entity& entity = m_pEngine->GetEntity(pAmplitudeEntity->m_uiAmId); entity.Valid())
  {
    entity.SetLocation(Utils::ezVec3ToAmVec3(Transform.m_vPosition));
    entity.SetOrientation(Utils::ezVec3ToAmVec3(-Transform.m_vForward), Utils::ezVec3ToAmVec3(Transform.m_vUp));
  }

  return EZ_SUCCESS;
}

ezResult ezAmplitude::LoadTrigger(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTriggerData* pTriggerData, ezAudioSystemEventData* pEventData)
{
  // Amplitude doesn't support loading triggers

  return EZ_SUCCESS;
}

ezResult ezAmplitude::ActivateTrigger(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTriggerData* pTriggerData, ezAudioSystemEventData* pEventData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const auto* const pAmplitudeTrigger = ezDynamicCast<const ezAmplitudeAudioTriggerData*>(pTriggerData);
  if (pAmplitudeTrigger == nullptr)
    return EZ_FAILURE;

  auto* pAmplitudeEvent = ezDynamicCast<ezAmplitudeAudioEventData*>(pEventData);
  if (pAmplitudeEvent == nullptr)
    return EZ_FAILURE;

  ezResult result = EZ_FAILURE;

  Amplitude::AmEntityID entityId = 0;

  // if (amplitudeEntity->m_bHasPosition)
  // {
  // Positioned entities
  entityId = pAmplitudeEntity->m_uiAmId;
  // }

  const Amplitude::Entity& entity = m_pEngine->GetEntity(entityId);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  if (entityId != Amplitude::kAmInvalidObjectId && !entity.Valid())
  {
    ezLog::Error("Unable to find entity with ID: {0}", entityId);
  }
#endif

  if (Amplitude::EventHandle const event = m_pEngine->GetEventHandle(pAmplitudeTrigger->m_uiAmId))
  {
    if (const Amplitude::EventCanceler& canceler = m_pEngine->Trigger(event, entity); canceler.Valid())
    {
      pAmplitudeEvent->m_eState = ezAudioSystemEventState::Playing;
      pAmplitudeEvent->m_EventCanceler = canceler;
      result = EZ_SUCCESS;
    }
  }
  else
  {
    ezLog::Error("[Amplitude] Unable to activate a trigger, the associated Amplitude event with ID {0} has not been found in loaded banks.", pAmplitudeTrigger->m_uiAmId);
  }

  return result;
}

ezResult ezAmplitude::UnloadTrigger(ezAudioSystemEntityData* pEntityData, const ezAudioSystemTriggerData* pTriggerData)
{
  // Amplitude doesn't support unloading triggers

  return EZ_SUCCESS;
}

ezResult ezAmplitude::StopEvent(ezAudioSystemEntityData* pEntityData, const ezAudioSystemEventData* pEventData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const auto* const pAmplitudeEvent = ezDynamicCast<const ezAmplitudeAudioEventData*>(pEventData);
  if (pAmplitudeEvent == nullptr)
    return EZ_FAILURE;

  ezResult result = EZ_FAILURE;

  switch (pAmplitudeEvent->m_eState)
  {
    case ezAudioSystemEventState::Playing:
    {
      if (pAmplitudeEvent->m_EventCanceler.Valid())
      {
        pAmplitudeEvent->m_EventCanceler.Cancel();
        result = EZ_SUCCESS;
        ezLog::Success("[Amplitude] Successfully stopped the event.");
      }
      else
      {
        ezLog::Error("[Amplitude] Encountered a running event without a valid canceler.");
      }
      break;
    }
    default:
    {
      ezLog::Warning("[Amplitude] Stopping an event of this type is not supported yet");
      break;
    }
  }

  return result;
}

ezResult ezAmplitude::StopAllEvents(ezAudioSystemEntityData* pEntityData)
{
  // TODO: Stop all events
  return EZ_SUCCESS;
}

ezResult ezAmplitude::SetRtpc(ezAudioSystemEntityData* pEntityData, const ezAudioSystemRtpcData* pRtpcData, float fValue)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const auto* const pAmplitudeRtpc = ezDynamicCast<const ezAmplitudeAudioRtpcData*>(pRtpcData);
  if (pAmplitudeRtpc == nullptr)
    return EZ_FAILURE;

  Amplitude::RtpcHandle const rtpc = m_pEngine->GetRtpcHandle(pAmplitudeRtpc->m_uiAmId);

  if (rtpc == nullptr)
    return EZ_FAILURE;

  rtpc->SetValue(fValue);

  return EZ_SUCCESS;
}

ezResult ezAmplitude::ResetRtpc(ezAudioSystemEntityData* pEntityData, const ezAudioSystemRtpcData* pRtpcData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const auto* const pAmplitudeRtpc = ezDynamicCast<const ezAmplitudeAudioRtpcData*>(pRtpcData);
  if (pAmplitudeRtpc == nullptr)
    return EZ_FAILURE;

  Amplitude::RtpcHandle const rtpc = m_pEngine->GetRtpcHandle(pAmplitudeRtpc->m_uiAmId);

  if (rtpc == nullptr)
    return EZ_FAILURE;

  rtpc->Reset();

  return EZ_SUCCESS;
}

ezResult ezAmplitude::SetSwitchState(ezAudioSystemEntityData* pEntityData, const ezAudioSystemSwitchStateData* pSwitchStateData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const auto* const pAmplitudeSwitch = ezDynamicCast<const ezAmplitudeAudioSwitchStateData*>(pSwitchStateData);
  if (pAmplitudeSwitch == nullptr)
    return EZ_FAILURE;

  Amplitude::SwitchHandle const _switch = m_pEngine->GetSwitchHandle(pAmplitudeSwitch->m_uiSwitchId);

  if (_switch == nullptr)
    return EZ_FAILURE;

  _switch->SetState(pAmplitudeSwitch->m_uiSwitchStateId);

  return EZ_SUCCESS;
}

ezResult ezAmplitude::SetObstructionAndOcclusion(ezAudioSystemEntityData* pEntityData, float fObstruction, float fOcclusion)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const Amplitude::Entity& entity = m_pEngine->GetEntity(pAmplitudeEntity->m_uiAmId);

  if (!entity.Valid())
    return EZ_FAILURE;

  entity.SetObstruction(fObstruction);
  entity.SetOcclusion(fOcclusion);

  return EZ_SUCCESS;
}

ezResult ezAmplitude::SetEnvironmentAmount(ezAudioSystemEntityData* pEntityData, const ezAudioSystemEnvironmentData* pEnvironmentData, float fAmount)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeEntity = ezDynamicCast<ezAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return EZ_FAILURE;

  const auto* const pAmplitudeEnvironment = ezDynamicCast<const ezAmplitudeAudioEnvironmentData*>(pEnvironmentData);
  if (pAmplitudeEnvironment == nullptr)
    return EZ_FAILURE;

  const Amplitude::Environment& environment = m_pEngine->AddEnvironment(pAmplitudeEnvironment->m_uiAmId);
  environment.SetEffect(pAmplitudeEnvironment->m_uiEffectId);

  if (!environment.Valid())
    return EZ_FAILURE;

  const Amplitude::Entity& entity = m_pEngine->GetEntity(pAmplitudeEntity->m_uiAmId);

  if (!entity.Valid())
    return EZ_FAILURE;

  entity.SetEnvironmentFactor(environment.GetId(), fAmount);

  return EZ_SUCCESS;
}

ezResult ezAmplitude::AddListener(ezAudioSystemListenerData* pListenerData, const char* szListenerName)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeListener = ezDynamicCast<ezAmplitudeAudioListenerData*>(pListenerData);
  if (pAmplitudeListener == nullptr)
    return EZ_FAILURE;

  const Amplitude::Listener& listener = m_pEngine->AddListener(pAmplitudeListener->m_uiAmId);

  return listener.Valid() ? EZ_SUCCESS : EZ_FAILURE;
}

ezResult ezAmplitude::ResetListener(ezAudioSystemListenerData* pListenerData)
{
  // Nothing to reset for now
  return EZ_SUCCESS;
}

ezResult ezAmplitude::RemoveListener(ezAudioSystemListenerData* pListenerData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeListener = ezDynamicCast<ezAmplitudeAudioListenerData*>(pListenerData);
  if (pAmplitudeListener == nullptr)
    return EZ_FAILURE;

  m_pEngine->RemoveListener(pAmplitudeListener->m_uiAmId);
  const Amplitude::Listener& listener = m_pEngine->GetListener(pAmplitudeListener->m_uiAmId);

  return listener.Valid() ? EZ_FAILURE : EZ_SUCCESS;
}

ezResult ezAmplitude::SetListenerTransform(ezAudioSystemListenerData* pListenerData, const ezAudioSystemTransform& Transform)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeListener = ezDynamicCast<ezAmplitudeAudioListenerData*>(pListenerData);
  if (pAmplitudeListener == nullptr)
    return EZ_FAILURE;

  if (const Amplitude::Listener& listener = m_pEngine->GetListener(pAmplitudeListener->m_uiAmId); listener.Valid())
  {
    listener.SetLocation(Utils::ezVec3ToAmVec3(Transform.m_vPosition));
    listener.SetOrientation(Utils::ezVec3ToAmVec3(-Transform.m_vForward), Utils::ezVec3ToAmVec3(Transform.m_vUp));
  }

  return EZ_SUCCESS;
}

ezResult ezAmplitude::LoadBank(ezAudioSystemBankData* pBankData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeBank = ezDynamicCast<ezAmplitudeAudioSoundBankData*>(pBankData);
  if (pAmplitudeBank == nullptr)
    return EZ_FAILURE;

  if (!m_pEngine->LoadSoundBank(AM_STRING_TO_OS_STRING(pAmplitudeBank->m_sFileName.GetData())))
  {
    ezLog::Error("[Amplitude] Could not load sound bank '{0}'.", pAmplitudeBank->m_sFileName);
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezAmplitude::UnloadBank(ezAudioSystemBankData* pBankData)
{
  if (!m_bInitialized)
    return EZ_FAILURE;

  const auto* const pAmplitudeBank = ezDynamicCast<ezAmplitudeAudioSoundBankData*>(pBankData);
  if (pAmplitudeBank == nullptr)
    return EZ_FAILURE;

  if (pAmplitudeBank->m_uiAmId == Amplitude::kAmInvalidObjectId)
    return EZ_FAILURE;

  m_pEngine->UnloadSoundBank(pAmplitudeBank->m_uiAmId);

  return EZ_SUCCESS;
}

ezAudioSystemEntityData* ezAmplitude::CreateWorldEntity(ezAudioSystemDataID uiEntityId)
{
  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioEntityData, uiEntityId, false);
}

ezAudioSystemEntityData* ezAmplitude::CreateEntityData(ezAudioSystemDataID uiEntityId)
{
  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioEntityData, uiEntityId);
}

ezResult ezAmplitude::DestroyEntityData(ezAudioSystemEntityData* pEntityData)
{
  if (pEntityData == nullptr || !pEntityData->IsInstanceOf<ezAmplitudeAudioEntityData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pEntityData);
  return EZ_SUCCESS;
}

ezAudioSystemListenerData* ezAmplitude::CreateListenerData(ezAudioSystemDataID uiListenerId)
{
  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioListenerData, uiListenerId);
}

ezResult ezAmplitude::DestroyListenerData(ezAudioSystemListenerData* pListenerData)
{
  if (pListenerData == nullptr || !pListenerData->IsInstanceOf<ezAmplitudeAudioListenerData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pListenerData);
  return EZ_SUCCESS;
}

ezAudioSystemEventData* ezAmplitude::CreateEventData(ezAudioSystemDataID uiEventId)
{
  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioEventData, uiEventId);
}

ezResult ezAmplitude::ResetEventData(ezAudioSystemEventData* pEventData)
{
  if (pEventData == nullptr || !pEventData->IsInstanceOf<ezAmplitudeAudioEventData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pEventData);
  return EZ_SUCCESS;
}

ezResult ezAmplitude::DestroyEventData(ezAudioSystemEventData* pEventData)
{
  if (pEventData == nullptr || !pEventData->IsInstanceOf<ezAmplitudeAudioEventData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pEventData);
  return EZ_SUCCESS;
}

ezAudioSystemBankData* ezAmplitude::DeserializeBankEntry(ezStreamReader* pBankEntry)
{
  if (pBankEntry == nullptr)
    return nullptr;

  Amplitude::AmBankID uiAmId;
  *pBankEntry >> uiAmId;

  ezString sFileName;
  *pBankEntry >> sFileName;

  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioSoundBankData, uiAmId, sFileName);
}

ezResult ezAmplitude::DestroyBank(ezAudioSystemBankData* pBankData)
{
  if (pBankData == nullptr || !pBankData->IsInstanceOf<ezAmplitudeAudioSoundBankData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pBankData);
  return EZ_SUCCESS;
}

ezAudioSystemTriggerData* ezAmplitude::DeserializeTriggerEntry(ezStreamReader* pTriggerEntry) const
{
  if (pTriggerEntry == nullptr)
    return nullptr;

  Amplitude::AmEventID uiAmId;
  *pTriggerEntry >> uiAmId;

  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioTriggerData, uiAmId);
}

ezResult ezAmplitude::DestroyTriggerData(ezAudioSystemTriggerData* pTriggerData)
{
  if (pTriggerData == nullptr || !pTriggerData->IsInstanceOf<ezAmplitudeAudioTriggerData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pTriggerData);
  return EZ_SUCCESS;
}

ezAudioSystemRtpcData* ezAmplitude::DeserializeRtpcEntry(ezStreamReader* pRtpcEntry) const
{
  if (pRtpcEntry == nullptr)
    return nullptr;

  Amplitude::AmEventID uiAmId;
  *pRtpcEntry >> uiAmId;

  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioRtpcData, uiAmId);
}

ezResult ezAmplitude::DestroyRtpcData(ezAudioSystemRtpcData* pRtpcData)
{
  if (pRtpcData == nullptr || !pRtpcData->IsInstanceOf<ezAmplitudeAudioRtpcData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pRtpcData);
  return EZ_SUCCESS;
}

ezAudioSystemSwitchStateData* ezAmplitude::DeserializeSwitchStateEntry(ezStreamReader* pSwitchStateEntry) const
{
  if (pSwitchStateEntry == nullptr)
    return nullptr;

  Amplitude::AmSwitchID uiSwitchId;
  *pSwitchStateEntry >> uiSwitchId;

  Amplitude::AmObjectID uiSwitchStateId;
  *pSwitchStateEntry >> uiSwitchStateId;

  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioSwitchStateData, uiSwitchId, uiSwitchStateId);
}

ezResult ezAmplitude::DestroySwitchStateData(ezAudioSystemSwitchStateData* pSwitchStateData)
{
  if (pSwitchStateData == nullptr || !pSwitchStateData->IsInstanceOf<ezAmplitudeAudioSwitchStateData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pSwitchStateData);
  return EZ_SUCCESS;
}

ezAudioSystemEnvironmentData* ezAmplitude::DeserializeEnvironmentEntry(ezStreamReader* pEnvironmentEntry) const
{
  if (pEnvironmentEntry == nullptr)
    return nullptr;

  Amplitude::AmEnvironmentID uiEnvironmentId;
  *pEnvironmentEntry >> uiEnvironmentId;

  Amplitude::AmEffectID uiEffectId;
  *pEnvironmentEntry >> uiEffectId;

  return EZ_NEW(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), ezAmplitudeAudioEnvironmentData, uiEnvironmentId, uiEffectId);
}

ezResult ezAmplitude::DestroyEnvironmentData(ezAudioSystemEnvironmentData* pEnvironmentData)
{
  if (pEnvironmentData == nullptr || !pEnvironmentData->IsInstanceOf<ezAmplitudeAudioEnvironmentData>())
    return EZ_FAILURE;

  EZ_DELETE(ezAudioMiddlewareAllocatorWrapper::GetAllocator(), pEnvironmentData);
  return EZ_SUCCESS;
}

ezResult ezAmplitude::SetLanguage(const char* szLanguage)
{
  return EZ_SUCCESS;
}

const char* ezAmplitude::GetMiddlewareName() const
{
  return s_szAmplitudeMiddlewareName;
}

float ezAmplitude::GetMasterGain() const
{
  return m_pEngine->GetMasterGain();
}

bool ezAmplitude::GetMute() const
{
  return m_pEngine->IsMuted();
}

void ezAmplitude::OnMasterGainChange(float fGain)
{
  // Master Volume
  m_pEngine->SetMasterGain(fGain);
}

void ezAmplitude::OnMuteChange(bool bMute)
{
  // Mute
  m_pEngine->SetMute(bMute);
}

void ezAmplitude::OnLoseFocus()
{
}

void ezAmplitude::OnGainFocus()
{
}

void ezAmplitude::Update(ezTime delta)
{
  if (m_pEngine == nullptr)
    return;

  EZ_ASSERT_DEV(m_pData != nullptr, "UpdateSound() should not be called at this time.");

  m_pEngine->AdvanceFrame(delta.AsFloatInSeconds());
}

void ezAmplitude::DetectPlatform() const
{
  if (!m_pData->m_sPlatform.IsEmpty())
    return;

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
  m_pData->m_sPlatform = "Desktop";

#elif EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)
  m_pData->m_sPlatform = "Desktop"; /// \todo Need to detect mobile device mode

#elif EZ_ENABLED(EZ_PLATFORM_LINUX)
  m_pData->m_sPlatform = "Desktop"; /// \todo Need to detect mobile device mode (Android)

#elif EZ_ENABLED(EZ_PLATFORM_OSX)
  m_pData->m_sPlatform = "Desktop";

#elif EZ_ENABLED(EZ_PLATFORM_IOS)
  m_pData->m_sPlatform = "Mobile";

#elif EZ_ENABLED(EZ_PLATFORM_ANDROID)
  m_pData->m_sPlatform = "Mobile";

#elif
#  error "Unknown Platform"

#endif
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_AmplitudeAudioSingleton);
