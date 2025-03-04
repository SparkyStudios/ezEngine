#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAsset.h>

#include <EditorFramework/Assets/AssetCurator.h>

#include <Foundation/IO/OSFile.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetEntry, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetEntry>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Name", m_sName),
    EZ_ENUM_MEMBER_PROPERTY("Type", ezAmplitudeAudioControlType, m_Type)->AddAttributes(new ezDefaultValueAttribute(ezAmplitudeAudioControlType::Invalid), new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("Control", m_sControlFile)->AddAttributes(new ezFileBrowserAttribute("Select Audio System Control", "*.ezAudioSystemControl")),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetTriggerEntry, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetTriggerEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetRtpcEntry, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetRtpcEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetSwitchEntry, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetSwitchEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetEnvironmentEntry, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetEnvironmentEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetSoundBankEntry, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetSoundBankEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAsset, 2, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAsset>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("Triggers", m_TriggerEntries),
    EZ_ARRAY_MEMBER_PROPERTY("RTPCs", m_RtpcEntries),
    EZ_ARRAY_MEMBER_PROPERTY("SwitchStates", m_SwitchEntries),
    EZ_ARRAY_MEMBER_PROPERTY("Environments", m_EnvironmentEntries),
    EZ_ARRAY_MEMBER_PROPERTY("SoundBanks", m_SoundBankEntries),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetDocument, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAmplitudeAudioControlCollectionAssetTriggerEntry::ezAmplitudeAudioControlCollectionAssetTriggerEntry()
  : ezAmplitudeAudioControlCollectionAssetEntry()
{
  m_Type = ezAmplitudeAudioControlType::Trigger;
}

ezAmplitudeAudioControlCollectionAssetRtpcEntry::ezAmplitudeAudioControlCollectionAssetRtpcEntry()
  : ezAmplitudeAudioControlCollectionAssetEntry()
{
  m_Type = ezAmplitudeAudioControlType::Rtpc;
}

ezAmplitudeAudioControlCollectionAssetSwitchEntry::ezAmplitudeAudioControlCollectionAssetSwitchEntry()
{
  m_Type = ezAmplitudeAudioControlType::SwitchState;
}

ezAmplitudeAudioControlCollectionAssetEnvironmentEntry::ezAmplitudeAudioControlCollectionAssetEnvironmentEntry()
{
  m_Type = ezAmplitudeAudioControlType::Environment;
}

ezAmplitudeAudioControlCollectionAssetSoundBankEntry::ezAmplitudeAudioControlCollectionAssetSoundBankEntry()
{
  m_Type = ezAmplitudeAudioControlType::SoundBank;
}

ezAmplitudeAudioControlCollectionAssetDocument::ezAmplitudeAudioControlCollectionAssetDocument(ezStringView szDocumentPath)
  : ezSimpleAssetDocument<ezAmplitudeAudioControlCollectionAsset>(szDocumentPath, ezAssetDocEngineConnection::None)
{
}

void ezAmplitudeAudioControlCollectionAssetDocument::UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const
{
  SUPER::UpdateAssetDocumentInfo(pInfo);

  const ezAmplitudeAudioControlCollectionAsset* pProp = GetProperties();

  for (const auto& e : pProp->m_TriggerEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_TransformDependencies.Insert(e.m_sControlFile);
  }

  for (const auto& e : pProp->m_RtpcEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_TransformDependencies.Insert(e.m_sControlFile);
  }

  for (const auto& e : pProp->m_SwitchEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_TransformDependencies.Insert(e.m_sControlFile);
  }

  for (const auto& e : pProp->m_EnvironmentEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_TransformDependencies.Insert(e.m_sControlFile);
  }

  for (const auto& e : pProp->m_SoundBankEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_TransformDependencies.Insert(e.m_sControlFile);
  }
}

ezTransformStatus ezAmplitudeAudioControlCollectionAssetDocument::InternalTransformAsset(ezStreamWriter& stream, ezStringView szOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags)
{
  ezAmplitudeAudioControlCollectionAsset* pProp = GetProperties();

  ezAmplitudeAudioControlCollectionResourceDescriptor descriptor;

  for (auto& e : pProp->m_TriggerEntries)
  {
    e.m_Type = ezAmplitudeAudioControlType::Trigger;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  for (auto& e : pProp->m_RtpcEntries)
  {
    e.m_Type = ezAmplitudeAudioControlType::Rtpc;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  for (auto& e : pProp->m_SwitchEntries)
  {
    e.m_Type = ezAmplitudeAudioControlType::SwitchState;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  for (auto& e : pProp->m_EnvironmentEntries)
  {
    e.m_Type = ezAmplitudeAudioControlType::Environment;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  for (auto& e : pProp->m_SoundBankEntries)
  {
    e.m_Type = ezAmplitudeAudioControlType::SoundBank;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  descriptor.Save(stream);
  return {EZ_SUCCESS};
}

ezResult ezAmplitudeAudioControlCollectionAssetDocument::TransformAssetEntry(const ezAmplitudeAudioControlCollectionAssetEntry& entry, ezAmplitudeAudioControlCollectionResourceDescriptor& descriptor) const
{
  if (entry.m_sControlFile.IsEmpty() || entry.m_Type == ezAmplitudeAudioControlType::Invalid)
    return EZ_FAILURE;

  {
    ezStringBuilder sAssetFile = entry.m_sControlFile;
    if (!ezQtEditorApp::GetSingleton()->MakeDataDirectoryRelativePathAbsolute(sAssetFile))
    {
      ezLog::Warning("Failed to make audio control path absolute: '{0}'", entry.m_sControlFile);
      return EZ_FAILURE;
    }
  }

  ezAmplitudeAudioControlCollectionEntry e;
  e.m_sName = entry.m_sName;
  e.m_Type = entry.m_Type;
  e.m_sControlFile = entry.m_sControlFile;

  descriptor.m_Entries.PushBack(e);
  return EZ_SUCCESS;
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(EditorPluginAmplitudeAudio, EditorPluginAmplitudeAudio_Assets_AmplitudeAudioControlCollectionAsset);
