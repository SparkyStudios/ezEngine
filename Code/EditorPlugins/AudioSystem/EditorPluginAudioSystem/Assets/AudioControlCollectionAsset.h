#pragma once

#include <AudioSystemPlugin/Core/AudioSystemData.h>
#include <AudioSystemPlugin/Resources/AudioControlCollectionResource.h>

#include <EditorFramework/Assets/SimpleAssetDocument.h>

class ezAudioControlCollectionAssetEntry : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioControlCollectionAssetEntry, ezReflectedClass);

public:
  ezString m_sName;
  ezEnum<ezAudioSystemControlType> m_Type;
  ezString m_sControlFile;
};

class ezAudioControlCollectionAssetTriggerEntry : public ezAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioControlCollectionAssetTriggerEntry, ezAudioControlCollectionAssetEntry);

public:
  ezAudioControlCollectionAssetTriggerEntry();
};

class ezAudioControlCollectionAssetRtpcEntry : public ezAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioControlCollectionAssetRtpcEntry, ezAudioControlCollectionAssetEntry);

public:
  ezAudioControlCollectionAssetRtpcEntry();
};

class ezAudioControlCollectionAssetSwitchEntry : public ezAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioControlCollectionAssetSwitchEntry, ezAudioControlCollectionAssetEntry);

public:
  ezAudioControlCollectionAssetSwitchEntry();
};

class ezAudioControlCollectionAsset : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioControlCollectionAsset, ezReflectedClass);

public:
  ezDynamicArray<ezAudioControlCollectionAssetTriggerEntry> m_TriggerEntries;
  ezDynamicArray<ezAudioControlCollectionAssetRtpcEntry> m_RtpcEntries;
  ezDynamicArray<ezAudioControlCollectionAssetSwitchEntry> m_SwitchEntries;
};

class ezAudioControlCollectionAssetDocument : public ezSimpleAssetDocument<ezAudioControlCollectionAsset>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAudioControlCollectionAssetDocument, ezSimpleAssetDocument<ezAudioControlCollectionAsset>);

public:
  ezAudioControlCollectionAssetDocument(const char* szDocumentPath);

protected:
  void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
  ezStatus InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;

private:
  ezResult TransformAssetEntry(const ezAudioControlCollectionAssetEntry& entry, ezAudioControlCollectionResourceDescriptor& descriptor) const;
};
