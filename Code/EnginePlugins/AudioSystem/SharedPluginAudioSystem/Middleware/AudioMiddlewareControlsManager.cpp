#include <SharedPluginAudioSystem/SharedPluginAudioSystemPCH.h>

#include <SharedPluginAudioSystem/Middleware/AudioMiddlewareControlsManager.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

ezResult ezAudioMiddlewareControlsManager::CreateTriggerControl(const char* szControlName, const ezAudioSystemTriggerData* pControlData)
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

  // Serialize the trigger data. This method is implemented by the audio middleware.
  if (SerializeTriggerControl(&file, pControlData).Succeeded())
  {
    file.Close();
    return EZ_SUCCESS;
  }

  file.Close();
  return EZ_FAILURE;
}

ezResult ezAudioMiddlewareControlsManager::CreateRtpcControl(const char* szControlName, const ezAudioSystemRtpcData* pControlData)
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

  // Serialize the trigger data. This method is implemented by the audio middleware.
  if (SerializeRtpcControl(&file, pControlData).Succeeded())
  {
    file.Close();
    return EZ_SUCCESS;
  }

  file.Close();
  return EZ_FAILURE;
}

ezResult ezAudioMiddlewareControlsManager::CreateSwitchStateControl(const char* szControlName, const ezAudioSystemSwitchStateData* pControlData)
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

  // Serialize the trigger data. This method is implemented by the audio middleware.
  if (SerializeSwitchStateControl(&file, pControlData).Succeeded())
  {
    file.Close();
    return EZ_SUCCESS;
  }

  file.Close();
  return EZ_FAILURE;
}

EZ_STATICLINK_FILE(SharedPluginAudioSystem, SharedPluginAudioSystem_Middleware_AudioMiddlewareControlsManager);
