#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Core/AmplitudeAudioControlsManager.h>

#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>
#include <AmplitudeAudioPlugin/Core/Common.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/JSONReader.h>
#include <Foundation/IO/OSFile.h>

EZ_IMPLEMENT_SINGLETON(ezAmplitudeAudioControlsManager);

ezAmplitudeAudioControlsManager::ezAmplitudeAudioControlsManager()
  : m_SingletonRegistrar(this)
{
}

ezResult ezAmplitudeAudioControlsManager::ReloadControls()
{
  ezStringBuilder assetsPath;
  if (ezFileSystem::ResolvePath(":project/Sounds/Amplitude/amplitude_assets", &assetsPath, nullptr).Failed())
  {
    ezLog::Error("No Amplitude assets directory available. Amplitude will be deactivated.");
    return EZ_FAILURE;
  }

  ezStringBuilder projectPath;
  if (ezFileSystem::ResolvePath(":project/Sounds/Amplitude/amplitude_project", &projectPath, nullptr).Failed())
  {
    ezLog::Error("No Amplitude project directory available. Amplitude will be deactivated.");
    return EZ_FAILURE;
  }

  ezFileSystem::AddDataDirectory(">project/Sounds/ATL/Amplitude", "ATL", "atl", ezFileSystem::AllowWrites).IgnoreResult();

  {
    ezStringBuilder basePath(projectPath);
    basePath.AppendPath(kEventsFolder);

    if (LoadControlsInFolder(basePath, eAMCT_AMPLITUDE_EVENT).Failed())
      return EZ_FAILURE;
  }

  {
    ezStringBuilder basePath(projectPath);
    basePath.AppendPath(kRtpcFolder);

    if (LoadControlsInFolder(basePath, eAMCT_AMPLITUDE_RTPC).Failed())
      return EZ_FAILURE;
  }

  {
    ezStringBuilder basePath(projectPath);
    basePath.AppendPath(kSwitchesFolder);

    if (LoadControlsInFolder(basePath, eAMCT_AMPLITUDE_SWITCH).Failed())
      return EZ_FAILURE;
  }

  {
    ezStringBuilder basePath(projectPath);
    basePath.AppendPath(kEnvironmentsFolder);

    if (LoadControlsInFolder(basePath, eAMCT_AMPLITUDE_ENVIRONMENT).Failed())
      return EZ_FAILURE;
  }

  LoadSoundBanks(projectPath, kSoundBanksFolder);

  return EZ_SUCCESS;
}

ezResult ezAmplitudeAudioControlsManager::SerializeTriggerControl(ezStreamWriter* pStream, const ezAudioSystemTriggerData* pControlData)
{
  if (pStream == nullptr || pControlData == nullptr)
    return EZ_FAILURE;

  if (const auto* const pAmplitudeAudioTriggerData = ezDynamicCast<const ezAmplitudeAudioTriggerData*>(pControlData); pAmplitudeAudioTriggerData != nullptr)
  {
    *pStream << pAmplitudeAudioTriggerData->m_uiAmId;
    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::SerializeRtpcControl(ezStreamWriter* pStream, const ezAudioSystemRtpcData* pControlData)
{
  if (pStream == nullptr || pControlData == nullptr)
    return EZ_FAILURE;

  if (const auto* const pAmplitudeAudioRtpcData = ezDynamicCast<const ezAmplitudeAudioRtpcData*>(pControlData); pAmplitudeAudioRtpcData != nullptr)
  {
    *pStream << pAmplitudeAudioRtpcData->m_uiAmId;
    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::SerializeSwitchStateControl(ezStreamWriter* pStream, const ezAudioSystemSwitchStateData* pControlData)
{
  if (pStream == nullptr || pControlData == nullptr)
    return EZ_FAILURE;

  if (const auto* const pAmplitudeAudioSwitchStateData = ezDynamicCast<const ezAmplitudeAudioSwitchStateData*>(pControlData); pAmplitudeAudioSwitchStateData != nullptr)
  {
    *pStream << pAmplitudeAudioSwitchStateData->m_uiSwitchId;
    *pStream << pAmplitudeAudioSwitchStateData->m_uiSwitchStateId;
    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::SerializeEnvironmentControl(ezStreamWriter* pStream, const ezAudioSystemEnvironmentData* pControlData)
{
  if (pStream == nullptr || pControlData == nullptr)
    return EZ_FAILURE;

  if (const auto* const pAmplitudeAudioEnvironmentData = ezDynamicCast<const ezAmplitudeAudioEnvironmentData*>(pControlData); pAmplitudeAudioEnvironmentData != nullptr)
  {
    *pStream << pAmplitudeAudioEnvironmentData->m_uiAmId;
    *pStream << pAmplitudeAudioEnvironmentData->m_uiEffectId;
    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::CreateTriggerControl(const char* szControlName, const ezAudioSystemTriggerData* pControlData)
{
  ezStringBuilder sbOutputFile;
  sbOutputFile.Format(":atl/Triggers/{0}.ezAudioSystemControl", szControlName);

  ezStringBuilder sbAssetPath;
  if (ezFileSystem::ResolvePath(sbOutputFile, &sbAssetPath, nullptr).Failed())
    return EZ_FAILURE;

  ezFileWriter file;
  if (file.Open(sbAssetPath, 256).Failed())
    return EZ_FAILURE;

  // Set the control type
  file << ezAudioSystemControlType::Trigger;

  // Serialize the control data
  if (SerializeTriggerControl(&file, pControlData).Succeeded())
    return EZ_SUCCESS;

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::CreateRtpcControl(const char* szControlName, const ezAudioSystemRtpcData* pControlData)
{
  ezStringBuilder sbOutputFile;
  sbOutputFile.Format(":atl/Rtpcs/{0}.ezAudioSystemControl", szControlName);

  ezStringBuilder sbAssetPath;
  if (ezFileSystem::ResolvePath(sbOutputFile, &sbAssetPath, nullptr).Failed())
    return EZ_FAILURE;

  ezFileWriter file;
  if (file.Open(sbAssetPath, 256).Failed())
    return EZ_FAILURE;

  // Set the control type
  file << ezAudioSystemControlType::Rtpc;

  // Serialize the control data
  if (SerializeRtpcControl(&file, pControlData).Succeeded())
    return EZ_SUCCESS;

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::CreateSwitchStateControl(const char* szControlName, const ezAudioSystemSwitchStateData* pControlData)
{
  ezStringBuilder sbOutputFile;
  sbOutputFile.Format(":atl/SwitchStates/{0}.ezAudioSystemControl", szControlName);

  ezStringBuilder sbAssetPath;
  if (ezFileSystem::ResolvePath(sbOutputFile, &sbAssetPath, nullptr).Failed())
    return EZ_FAILURE;

  ezFileWriter file;
  if (file.Open(sbAssetPath, 256).Failed())
    return EZ_FAILURE;

  // Set the control type
  file << ezAudioSystemControlType::SwitchState;

  // Serialize the control data
  if (SerializeSwitchStateControl(&file, pControlData).Succeeded())
    return EZ_SUCCESS;

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::CreateEnvironmentControl(const char* szControlName, const ezAudioSystemEnvironmentData* pControlData)
{
  ezStringBuilder sbOutputFile;
  sbOutputFile.Format(":atl/Environments/{0}.ezAudioSystemControl", szControlName);

  ezStringBuilder sbAssetPath;
  if (ezFileSystem::ResolvePath(sbOutputFile, &sbAssetPath, nullptr).Failed())
    return EZ_FAILURE;

  ezFileWriter file;
  if (file.Open(sbAssetPath, 256).Failed())
    return EZ_FAILURE;

  // Set the control type
  file << ezAudioSystemControlType::Environment;

  // Serialize the control data
  if (SerializeEnvironmentControl(&file, pControlData).Succeeded())
    return EZ_SUCCESS;

  return EZ_FAILURE;
}

void ezAmplitudeAudioControlsManager::LoadSoundBanks(const char* sRootFolder, const char* sSubPath)
{
  ezStringBuilder searchPath(sRootFolder);
  searchPath.AppendPath(sSubPath);

  ezFileSystemIterator fsIt;
  for (fsIt.StartSearch(searchPath, ezFileSystemIteratorFlags::ReportFilesRecursive); fsIt.IsValid(); fsIt.Next())
  {
    ezStringBuilder filePath = fsIt.GetCurrentPath();
    filePath.AppendPath(fsIt.GetStats().m_sName);

    // Read the asset into a memory buffer
    ezFileReader reader;
    if (!reader.Open(filePath).Succeeded())
    {
      ezLog::Error("Could not open sound bank file '{0}'.", filePath);
      continue;
    }

    ezJSONReader json;
    json.SetLogInterface(ezLog::GetThreadLocalLogSystem());

    if (json.Parse(reader).Succeeded())
    {
      //      LoadControl(json, eAMCT_AMPLITUDE_SOUND_BANK);
      //      ezLog::Info("Successfully parsed sound bank file '{0}'.", filePath);
    }
    else
    {
      ezLog::Error("Could not parse sound bank file '{0}'.", filePath);
    }
  }
}

ezResult ezAmplitudeAudioControlsManager::LoadControlsInFolder(const char* sFolderPath, AmplitudeControlType type)
{
  ezStringBuilder const searchPath(sFolderPath);

  ezFileSystemIterator fsIt;
  for (fsIt.StartSearch(searchPath, ezFileSystemIteratorFlags::ReportFilesRecursive); fsIt.IsValid(); fsIt.Next())
  {
    ezStringBuilder filePath = fsIt.GetCurrentPath();
    filePath.AppendPath(fsIt.GetStats().m_sName);

    // Read the asset into a memory buffer
    ezFileReader reader;
    if (!reader.Open(filePath).Succeeded())
    {
      ezLog::Error("Could not open control file '{0}'.", filePath);
      continue;
    }

    ezJSONReader json;
    json.SetLogInterface(ezLog::GetThreadLocalLogSystem());

    if (json.Parse(reader).Succeeded())
    {
      if (LoadControl(json.GetTopLevelObject(), type).Succeeded())
        return EZ_SUCCESS;
    }
    else
    {
      ezLog::Error("Could not parse control file '{0}'.", filePath);
      return EZ_FAILURE;
    }
  }

  return EZ_FAILURE;
}

ezResult ezAmplitudeAudioControlsManager::LoadControl(const ezVariantDictionary& json, AmplitudeControlType type)
{
  if (json.Contains("name"))
  {
    const ezVariant* name = json.GetValue("name");
    const ezVariant* id = json.GetValue("id");

    if (!name->CanConvertTo<ezString>() || !id->CanConvertTo<ezUInt64>())
      return EZ_FAILURE;

    const ezString controlName(name->Get<ezString>());

    switch (type)
    {
      case eAMCT_INVALID:
        break;

      case eAMCT_AMPLITUDE_EVENT:
      {
        ezAmplitudeAudioTriggerData* control = EZ_AUDIOSYSTEM_NEW(ezAmplitudeAudioTriggerData, id->Get<ezUInt64>());
        const ezResult result = CreateTriggerControl(controlName, control);
        EZ_AUDIOSYSTEM_DELETE(control);
        return result;
      }

      case eAMCT_AMPLITUDE_RTPC:
      {
        ezAmplitudeAudioRtpcData* control = EZ_AUDIOSYSTEM_NEW(ezAmplitudeAudioRtpcData, id->Get<ezUInt64>());
        const ezResult result = CreateRtpcControl(controlName, control);
        EZ_AUDIOSYSTEM_DELETE(control);
        return result;
      }

      case eAMCT_AMPLITUDE_SOUND_BANK:
        break;

      case eAMCT_AMPLITUDE_SWITCH:
      {
        if (json.Contains("states"))
        {
          const ezVariant* states = json.GetValue("states");

          if (!states->CanConvertTo<ezVariantArray>())
            return EZ_FAILURE;

          for (const auto& state : states->Get<ezVariantArray>())
          {
            if (!state.CanConvertTo<ezVariantDictionary>())
              continue;

            const auto& value = state.Get<ezVariantDictionary>();

            ezStringBuilder stateName(controlName);
            stateName.AppendFormat("_{0}", value.GetValue("name")->Get<ezString>());
            ezAmplitudeAudioSwitchStateData* control = EZ_AUDIOSYSTEM_NEW(ezAmplitudeAudioSwitchStateData, id->Get<ezUInt64>(), value.GetValue("id")->Get<ezUInt64>());
            const ezResult result = CreateSwitchStateControl(stateName, control);
            EZ_AUDIOSYSTEM_DELETE(control);
            if (result.Failed())
              return result;
          }
          return EZ_SUCCESS;
        }

        return EZ_FAILURE;
      }

      case eAMCT_AMPLITUDE_ENVIRONMENT:
      {
        ezAmplitudeAudioEnvironmentData* control = EZ_AUDIOSYSTEM_NEW(ezAmplitudeAudioEnvironmentData, id->Get<ezUInt64>(), json.GetValue("effect")->Get<ezUInt64>());
        const ezResult result = CreateEnvironmentControl(controlName, control);
        EZ_AUDIOSYSTEM_DELETE(control);
        return result;
      }

      case eAMCT_AMPLITUDE_SWITCH_STATE:
        break;

      case eAMCT_AMPLITUDE_BUS:
        break;

      case eAMCT_AMPLITUDE_EFFECT:
        break;
    }
  }

  return EZ_FAILURE;
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(EditorPluginAmplitudeAudio, EditorPluginAmplitudeAudio_AmplitudeAudioControlsManager);
