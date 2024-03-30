#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>
#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>

#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileReader.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionResource, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionResource>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezAmplitudeAudioControlCollectionResource);

void ezAmplitudeAudioControlCollectionResourceDescriptor::Save(ezStreamWriter& stream) const
{
  constexpr ezUInt8 uiVersion = 1;
  constexpr ezUInt8 uiIdentifier = 0xAC;
  const ezUInt32 uiNumResources = m_Entries.GetCount();

  stream << uiVersion;
  stream << uiIdentifier;
  stream << uiNumResources;

  for (ezUInt32 i = 0; i < uiNumResources; ++i)
  {
    ezDefaultMemoryStreamStorage storage(0, ezAudioSystemAllocatorWrapper::GetAllocator());

    {
      ezFileReader file;
      if (file.Open(m_Entries[i].m_sControlFile).Failed())
      {
        ezLog::Error("Could not open audio control file '{0}'", m_Entries[i].m_sControlFile);
        continue;
      }

      ezMemoryStreamWriter writer(&storage);
      ezUInt8 Temp[4 * 1024];
      while (true)
      {
        const ezUInt64 uiRead = file.ReadBytes(Temp, EZ_ARRAY_SIZE(Temp));

        if (uiRead == 0)
          break;

        writer.WriteBytes(Temp, uiRead).IgnoreResult();
      }
    }

    stream << m_Entries[i].m_sName;
    stream << m_Entries[i].m_Type;
    stream << m_Entries[i].m_sControlFile;
    stream << storage.GetStorageSize32();
    storage.CopyToStream(stream).IgnoreResult();
  }
}

void ezAmplitudeAudioControlCollectionResourceDescriptor::Load(ezStreamReader& stream)
{
  ezUInt8 uiVersion = 0;
  ezUInt8 uiIdentifier = 0;
  ezUInt32 uiNumResources = 0;

  stream >> uiVersion;
  stream >> uiIdentifier;
  stream >> uiNumResources;

  EZ_ASSERT_DEV(uiIdentifier == 0xAC, "File does not contain a valid ezAmplitudeAudioControlCollectionResourceDescriptor");
  EZ_ASSERT_DEV(uiVersion == 1, "Invalid file version {0}", uiVersion);

  m_Entries.SetCount(uiNumResources);

  for (ezUInt32 i = 0; i < uiNumResources; ++i)
  {
    ezUInt32 uiSize = 0;

    stream >> m_Entries[i].m_sName;
    stream >> m_Entries[i].m_Type;
    stream >> m_Entries[i].m_sControlFile;
    stream >> uiSize;

    m_Entries[i].m_pControlBufferStorage = EZ_AUDIOSYSTEM_NEW(ezDefaultMemoryStreamStorage, uiSize, ezAudioSystemAllocatorWrapper::GetAllocator());
    m_Entries[i].m_pControlBufferStorage->ReadAll(stream, uiSize);
  }
}


EZ_RESOURCE_IMPLEMENT_CREATEABLE(ezAmplitudeAudioControlCollectionResource, ezAmplitudeAudioControlCollectionResourceDescriptor)
{
  m_Collection = descriptor;

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Loaded;

  return res;
}

ezAmplitudeAudioControlCollectionResource::ezAmplitudeAudioControlCollectionResource()
  : ezResource(DoUpdate::OnAnyThread, 1)
{
}

void ezAmplitudeAudioControlCollectionResource::Register()
{
  if (m_bRegistered)
    return;

  for (const auto& entry : m_Collection.m_Entries)
  {
    if (entry.m_sName.IsEmpty() || entry.m_pControlBufferStorage == nullptr)
      continue;

    ezMemoryStreamReader reader(entry.m_pControlBufferStorage);

    switch (entry.m_Type)
    {
      case ezAmplitudeAudioControlType::Switch:
      case ezAmplitudeAudioControlType::Invalid:
        ezLog::Error("Unable to register an audio control. Encountered an invalid control type");
        break;

      case ezAmplitudeAudioControlType::Trigger:
        RegisterTrigger(entry.m_sName, &reader);
        break;

      case ezAmplitudeAudioControlType::Rtpc:
        RegisterRtpc(entry.m_sName, &reader);
        break;

      case ezAmplitudeAudioControlType::SwitchState:
        RegisterSwitchState(entry.m_sName, &reader);
        break;

      case ezAmplitudeAudioControlType::Environment:
        RegisterEnvironment(entry.m_sName, &reader);
        break;

      case ezAmplitudeAudioControlType::SoundBank:
        RegisterSoundBank(entry.m_sName, &reader);
        break;
    }
  }
}

void ezAmplitudeAudioControlCollectionResource::Unregister()
{
  if (!m_bRegistered)
    return;

  for (const auto& entry : m_Collection.m_Entries)
  {
    if (entry.m_sName.IsEmpty() || entry.m_pControlBufferStorage == nullptr)
      continue;

    switch (entry.m_Type)
    {
      case ezAmplitudeAudioControlType::Invalid:
      case ezAmplitudeAudioControlType::Switch:
        ezLog::Error("Unable to unregister an audio control. Encountered an invalid control type.");
        break;

      case ezAmplitudeAudioControlType::Trigger:
        UnregisterTrigger(entry.m_sName);
        break;

      case ezAmplitudeAudioControlType::Rtpc:
        UnregisterRtpc(entry.m_sName);
        break;

      case ezAmplitudeAudioControlType::SwitchState:
        UnregisterSwitchState(entry.m_sName);
        break;

      case ezAmplitudeAudioControlType::Environment:
        UnregisterEnvironment(entry.m_sName);
        break;

      case ezAmplitudeAudioControlType::SoundBank:
        UnregisterSoundBank(entry.m_sName);
        break;
    }
  }
}

const ezAmplitudeAudioControlCollectionResourceDescriptor& ezAmplitudeAudioControlCollectionResource::GetDescriptor() const
{
  return m_Collection;
}

void ezAmplitudeAudioControlCollectionResource::RegisterTrigger(const char* szTriggerName, const char* szControlFile)
{
  if (!ezAudioSystem::GetSingleton()->IsInitialized())
    return;

  ezFileReader file;
  if (!file.Open(szControlFile, 256).Succeeded())
  {
    ezLog::Error("Unable to register a trigger in the audio system: Could not open trigger control file '{0}'", szControlFile);
    return;
  }

  RegisterTrigger(szTriggerName, &file);
}

void ezAmplitudeAudioControlCollectionResource::RegisterTrigger(const char* szTriggerName, ezStreamReader* pStreamReader)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  auto* pAudioMiddleware = ezAmplitude::GetSingleton();
  if (pAudioMiddleware == nullptr)
  {
    ezLog::Error("Unable to register a trigger in the audio system: No audio middleware currently running.");
    return;
  }

  ezEnum<ezAmplitudeAudioControlType> type;
  *pStreamReader >> type;

  if (type != ezAmplitudeAudioControlType::Trigger)
  {
    ezLog::Error("Unable to register a trigger in the audio system: The control have an invalid file.");
    return;
  }

  ezAudioSystemTriggerData* pTriggerData = pAudioMiddleware->DeserializeTriggerEntry(pStreamReader);
  if (pTriggerData == nullptr)
  {
    ezLog::Error("Unable to register a trigger in the audio system: Could not deserialize control.");
    return;
  }

  const ezUInt32 uiTriggerId = ezHashHelper<const char*>::Hash(szTriggerName);
  pAudioSystem->RegisterTrigger(uiTriggerId, pTriggerData);
}

void ezAmplitudeAudioControlCollectionResource::UnregisterTrigger(const char* szTriggerName)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  const ezUInt32 uiTriggerId = ezHashHelper<const char*>::Hash(szTriggerName);
  pAudioSystem->UnregisterTrigger(uiTriggerId);
}

void ezAmplitudeAudioControlCollectionResource::RegisterRtpc(const char* szRtpcName, const char* szControlFile)
{
  if (!ezAudioSystem::GetSingleton()->IsInitialized())
    return;

  ezFileReader file;
  if (!file.Open(szControlFile, 256).Succeeded())
  {
    ezLog::Error("Unable to register a rtpc in the audio system: Could not open control file '{0}'", szControlFile);
    return;
  }

  RegisterRtpc(szRtpcName, &file);
}

void ezAmplitudeAudioControlCollectionResource::RegisterRtpc(const char* szRtpcName, ezStreamReader* pStreamReader)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  auto* pAudioMiddleware = ezAmplitude::GetSingleton();
  if (pAudioMiddleware == nullptr)
  {
    ezLog::Error("Unable to register a rtpc in the audio system: No audio middleware currently running.");
    return;
  }

  ezEnum<ezAmplitudeAudioControlType> type;
  *pStreamReader >> type;

  if (type != ezAmplitudeAudioControlType::Rtpc)
  {
    ezLog::Error("Unable to register a rtpc in the audio system: The control have an invalid file.");
    return;
  }

  ezAudioSystemRtpcData* pSystemRtpcData = pAudioMiddleware->DeserializeRtpcEntry(pStreamReader);
  if (pSystemRtpcData == nullptr)
  {
    ezLog::Error("Unable to register a rtpc in the audio system: Could not deserialize control.");
    return;
  }

  const ezUInt32 uiRtpcId = ezHashHelper<const char*>::Hash(szRtpcName);
  pAudioSystem->RegisterRtpc(uiRtpcId, pSystemRtpcData);
}

void ezAmplitudeAudioControlCollectionResource::UnregisterRtpc(const char* szRtpcName)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  const ezUInt32 uiRtpcId = ezHashHelper<const char*>::Hash(szRtpcName);
  pAudioSystem->UnregisterRtpc(uiRtpcId);
}

void ezAmplitudeAudioControlCollectionResource::RegisterSwitchState(const char* szSwitchStateName, const char* szControlFile)
{
  if (!ezAudioSystem::GetSingleton()->IsInitialized())
    return;

  ezFileReader file;
  if (!file.Open(szControlFile, 256).Succeeded())
  {
    ezLog::Error("Unable to register a switch state in the audio system: Could not open control file '{0}'", szControlFile);
    return;
  }

  RegisterSwitchState(szSwitchStateName, &file);
}

void ezAmplitudeAudioControlCollectionResource::RegisterSwitchState(const char* szSwitchStateName, ezStreamReader* pStreamReader)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  auto* pAudioMiddleware = ezAmplitude::GetSingleton();
  if (pAudioMiddleware == nullptr)
  {
    ezLog::Error("Unable to register a switch state in the audio system: No audio middleware currently running.");
    return;
  }

  ezEnum<ezAmplitudeAudioControlType> type;
  *pStreamReader >> type;

  if (type != ezAmplitudeAudioControlType::SwitchState)
  {
    ezLog::Error("Unable to register a switch state in the audio system: The control have an invalid file.");
    return;
  }

  ezAudioSystemSwitchStateData* pSwitchStateData = pAudioMiddleware->DeserializeSwitchStateEntry(pStreamReader);
  if (pSwitchStateData == nullptr)
  {
    ezLog::Error("Unable to register a switch state in the audio system: Could not deserialize control.");
    return;
  }

  const ezUInt32 uiSwitchStateId = ezHashHelper<const char*>::Hash(szSwitchStateName);
  pAudioSystem->RegisterSwitchState(uiSwitchStateId, pSwitchStateData);
}

void ezAmplitudeAudioControlCollectionResource::UnregisterSwitchState(const char* szSwitchStateName)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  const ezUInt32 uiSwitchStateId = ezHashHelper<const char*>::Hash(szSwitchStateName);
  pAudioSystem->UnregisterSwitchState(uiSwitchStateId);
}

void ezAmplitudeAudioControlCollectionResource::RegisterEnvironment(const char* szEnvironmentName, const char* szControlFile)
{
  if (!ezAudioSystem::GetSingleton()->IsInitialized())
    return;

  ezFileReader file;
  if (!file.Open(szControlFile, 256).Succeeded())
  {
    ezLog::Error("Unable to register an environment in the audio system: Could not open control file '{0}'", szControlFile);
    return;
  }

  RegisterSwitchState(szEnvironmentName, &file);
}

void ezAmplitudeAudioControlCollectionResource::RegisterEnvironment(const char* szEnvironmentName, ezStreamReader* pStreamReader)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  auto* pAudioMiddleware = ezAmplitude::GetSingleton();
  if (pAudioMiddleware == nullptr)
  {
    ezLog::Error("Unable to register an environment in the audio system: No audio middleware currently running.");
    return;
  }

  ezEnum<ezAmplitudeAudioControlType> type;
  *pStreamReader >> type;

  if (type != ezAmplitudeAudioControlType::Environment)
  {
    ezLog::Error("Unable to register an environment in the audio system: The control have an invalid file.");
    return;
  }

  ezAudioSystemEnvironmentData* pEnvironmentData = pAudioMiddleware->DeserializeEnvironmentEntry(pStreamReader);
  if (pEnvironmentData == nullptr)
  {
    ezLog::Error("Unable to register an environment in the audio system: Could not deserialize control.");
    return;
  }

  const ezUInt32 uiEnvironmentId = ezHashHelper<const char*>::Hash(szEnvironmentName);
  pAudioSystem->RegisterEnvironment(uiEnvironmentId, pEnvironmentData);
}

void ezAmplitudeAudioControlCollectionResource::UnregisterEnvironment(const char* szEnvironmentName)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  const ezUInt32 uiEnvironmentId = ezHashHelper<const char*>::Hash(szEnvironmentName);
  pAudioSystem->UnregisterEnvironment(uiEnvironmentId);
}

void ezAmplitudeAudioControlCollectionResource::RegisterSoundBank(const char* szBankName, const char* szControlFile)
{
  if (!ezAudioSystem::GetSingleton()->IsInitialized())
    return;

  ezFileReader file;
  if (!file.Open(szBankName, 256).Succeeded())
  {
    ezLog::Error("Unable to register a sound bank in the audio system: Could not open control file '{0}'", szControlFile);
    return;
  }

  RegisterSoundBank(szBankName, &file);
}

void ezAmplitudeAudioControlCollectionResource::RegisterSoundBank(const char* szBankName, ezStreamReader* pStreamReader)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  auto* pAudioMiddleware = ezAmplitude::GetSingleton();
  if (pAudioMiddleware == nullptr)
  {
    ezLog::Error("Unable to register a sound bank in the audio system: No audio middleware currently running.");
    return;
  }

  ezEnum<ezAmplitudeAudioControlType> type;
  *pStreamReader >> type;

  if (type != ezAmplitudeAudioControlType::SoundBank)
  {
    ezLog::Error("Unable to register a sound bank in the audio system: The control have an invalid file.");
    return;
  }

  ezAudioSystemBankData* pSoundBankData = pAudioMiddleware->DeserializeBankEntry(pStreamReader);
  if (pSoundBankData == nullptr)
  {
    ezLog::Error("Unable to register a sound bank in the audio system: Could not deserialize control.");
    return;
  }

  const ezUInt32 uiSoundBankId = ezHashHelper<const char*>::Hash(szBankName);
  pAudioSystem->RegisterSoundBank(uiSoundBankId, pSoundBankData);
}

void ezAmplitudeAudioControlCollectionResource::UnregisterSoundBank(const char* szBankName)
{
  auto* pAudioSystem = ezAudioSystem::GetSingleton();
  if (!pAudioSystem->IsInitialized())
    return;

  const ezUInt32 uiSoundBankId = ezHashHelper<const char*>::Hash(szBankName);
  pAudioSystem->UnregisterSoundBank(uiSoundBankId);
}

ezResourceLoadDesc ezAmplitudeAudioControlCollectionResource::UnloadData(Unload WhatToUnload)
{
  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Unloaded;

  {
    Unregister();

    for (auto& entry : m_Collection.m_Entries)
    {
      if (entry.m_pControlBufferStorage != nullptr)
      {
        entry.m_pControlBufferStorage->Clear();
        EZ_AUDIOSYSTEM_DELETE(entry.m_pControlBufferStorage);
      }
    }

    m_Collection.m_Entries.Clear();
    m_Collection.m_Entries.Compact();
  }

  return res;
}

ezResourceLoadDesc ezAmplitudeAudioControlCollectionResource::UpdateContent(ezStreamReader* pStream)
{
  EZ_LOG_BLOCK("ezAmplitudeAudioControlCollectionResource::UpdateContent", GetResourceDescription().GetData());

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (pStream == nullptr)
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  // Skip the absolute file path data that the standard file reader writes into the stream
  {
    ezStringBuilder sAbsFilePath;
    *pStream >> sAbsFilePath;
  }

  // Skip the asset file header at the start of the file
  {
    ezAssetFileHeader AssetHash;
    AssetHash.Read(*pStream).IgnoreResult();
  }

  // Load the asset file
  m_Collection.Load(*pStream);

  // Register asset controls in the audio system
  Register();

  res.m_State = ezResourceState::Loaded;
  return res;
}

void ezAmplitudeAudioControlCollectionResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
  out_NewMemoryUsage.m_uiMemoryCPU = m_Collection.m_Entries.GetHeapMemoryUsage();
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Resources_AudioControlCollectionResource);
