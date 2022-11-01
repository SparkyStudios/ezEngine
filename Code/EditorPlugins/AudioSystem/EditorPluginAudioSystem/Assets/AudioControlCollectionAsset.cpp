#include <EditorPluginAudioSystem/EditorPluginAudioSystemPCH.h>

#include <EditorPluginAudioSystem/Assets/AudioControlCollectionAsset.h>

#include <EditorFramework/Assets/AssetCurator.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioControlCollectionAssetEntry, 1, ezRTTIDefaultAllocator<ezAudioControlCollectionAssetEntry>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Name", m_sName),
    EZ_ENUM_MEMBER_PROPERTY("Type", ezAudioSystemControlType, m_Type)->AddAttributes(new ezDefaultValueAttribute(ezAudioSystemControlType::Invalid), new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("Control", m_sControlFile)->AddAttributes(new ezFileBrowserAttribute("Select Audio System Control", "*.ezAudioSystemControl")),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioControlCollectionAssetTriggerEntry, 1, ezRTTIDefaultAllocator<ezAudioControlCollectionAssetTriggerEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioControlCollectionAssetRtpcEntry, 1, ezRTTIDefaultAllocator<ezAudioControlCollectionAssetRtpcEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioControlCollectionAssetSwitchEntry, 1, ezRTTIDefaultAllocator<ezAudioControlCollectionAssetSwitchEntry>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioControlCollectionAsset, 2, ezRTTIDefaultAllocator<ezAudioControlCollectionAsset>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("Triggers", m_TriggerEntries),
    EZ_ARRAY_MEMBER_PROPERTY("RTPCs", m_RtpcEntries),
    EZ_ARRAY_MEMBER_PROPERTY("SwitchStates", m_SwitchEntries),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAudioControlCollectionAssetDocument, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAudioControlCollectionAssetTriggerEntry::ezAudioControlCollectionAssetTriggerEntry()
  : ezAudioControlCollectionAssetEntry()
{
  m_Type = ezAudioSystemControlType::Trigger;
}

ezAudioControlCollectionAssetRtpcEntry::ezAudioControlCollectionAssetRtpcEntry()
  : ezAudioControlCollectionAssetEntry()
{
  m_Type = ezAudioSystemControlType::Rtpc;
}

ezAudioControlCollectionAssetSwitchEntry::ezAudioControlCollectionAssetSwitchEntry()
{
  m_Type = ezAudioSystemControlType::SwitchState;
}

ezAudioControlCollectionAssetDocument::ezAudioControlCollectionAssetDocument(const char* szDocumentPath)
  : ezSimpleAssetDocument<ezAudioControlCollectionAsset>(szDocumentPath, ezAssetDocEngineConnection::None)
{
}

void ezAudioControlCollectionAssetDocument::UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const
{
  SUPER::UpdateAssetDocumentInfo(pInfo);

  const ezAudioControlCollectionAsset* pProp = GetProperties();

  for (const auto& e : pProp->m_TriggerEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_AssetTransformDependencies.Insert(e.m_sControlFile);
  }

  for (const auto& e : pProp->m_RtpcEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_AssetTransformDependencies.Insert(e.m_sControlFile);
  }

  for (const auto& e : pProp->m_SwitchEntries)
  {
    if (!e.m_sControlFile.IsEmpty())
      pInfo->m_AssetTransformDependencies.Insert(e.m_sControlFile);
  }
}

ezStatus ezAudioControlCollectionAssetDocument::InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags)
{
  ezAudioControlCollectionAsset* pProp = GetProperties();

  ezAudioControlCollectionResourceDescriptor descriptor;

  for (auto& e : pProp->m_TriggerEntries)
  {
    e.m_Type = ezAudioSystemControlType::Trigger;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  for (auto& e : pProp->m_RtpcEntries)
  {
    e.m_Type = ezAudioSystemControlType::Rtpc;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  for (auto& e : pProp->m_SwitchEntries)
  {
    e.m_Type = ezAudioSystemControlType::SwitchState;
    TransformAssetEntry(e, descriptor).IgnoreResult();
  }

  descriptor.Save(stream);
  return {EZ_SUCCESS};
}

ezResult ezAudioControlCollectionAssetDocument::TransformAssetEntry(const ezAudioControlCollectionAssetEntry& entry, ezAudioControlCollectionResourceDescriptor& descriptor) const
{
  if (entry.m_sControlFile.IsEmpty() || entry.m_Type == ezAudioSystemControlType::Invalid)
    return EZ_FAILURE;

  {
    ezStringBuilder sAssetFile = entry.m_sControlFile;
    if (!ezQtEditorApp::GetSingleton()->MakeDataDirectoryRelativePathAbsolute(sAssetFile))
    {
      ezLog::Warning("Failed to make audio control path absolute: '{0}'", entry.m_sControlFile);
      return EZ_FAILURE;
    }
  }

  ezAudioControlCollectionEntry e;
  e.m_sName = entry.m_sName;
  e.m_Type = entry.m_Type;
  e.m_sControlFile = entry.m_sControlFile;

  descriptor.m_Entries.PushBack(e);
  return EZ_SUCCESS;
}
