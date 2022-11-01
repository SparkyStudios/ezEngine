#pragma once

#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <EditorFramework/Assets/SimpleAssetDocument.h>

class ezAmplitudeAudioControlCollectionAssetEntry : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetEntry, ezReflectedClass);

public:
  ezString m_sName;
  ezEnum<ezAudioSystemControlType> m_Type;
  ezString m_sControlFile;
};

class ezAmplitudeAudioControlCollectionAssetTriggerEntry : public ezAmplitudeAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetTriggerEntry, ezAmplitudeAudioControlCollectionAssetEntry);

public:
  ezAmplitudeAudioControlCollectionAssetTriggerEntry();
};

class ezAmplitudeAudioControlCollectionAssetRtpcEntry : public ezAmplitudeAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetRtpcEntry, ezAmplitudeAudioControlCollectionAssetEntry);

public:
  ezAmplitudeAudioControlCollectionAssetRtpcEntry();
};

class ezAmplitudeAudioControlCollectionAssetSwitchEntry : public ezAmplitudeAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetSwitchEntry, ezAmplitudeAudioControlCollectionAssetEntry);

public:
  ezAmplitudeAudioControlCollectionAssetSwitchEntry();
};

class ezAmplitudeAudioControlCollectionAssetEnvironmentEntry : public ezAmplitudeAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetEnvironmentEntry, ezAmplitudeAudioControlCollectionAssetEntry);

public:
  ezAmplitudeAudioControlCollectionAssetEnvironmentEntry();
};

class ezAmplitudeAudioControlCollectionAsset : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAsset, ezReflectedClass);

public:
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetTriggerEntry> m_TriggerEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetRtpcEntry> m_RtpcEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetSwitchEntry> m_SwitchEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetEnvironmentEntry> m_EnvironmentEntries;
};

class ezAmplitudeAudioControlCollectionAssetDocument : public ezSimpleAssetDocument<ezAmplitudeAudioControlCollectionAsset>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetDocument, ezSimpleAssetDocument<ezAmplitudeAudioControlCollectionAsset>);

public:
  ezAmplitudeAudioControlCollectionAssetDocument(const char* szDocumentPath);

protected:
  void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
  ezStatus InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;

private:
  ezResult TransformAssetEntry(const ezAmplitudeAudioControlCollectionAssetEntry& entry, ezAmplitudeAudioControlCollectionResourceDescriptor& descriptor) const;
};
