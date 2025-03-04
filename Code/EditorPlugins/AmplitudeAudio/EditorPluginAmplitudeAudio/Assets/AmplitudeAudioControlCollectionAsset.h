#pragma once

#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>

#include <AudioSystemPlugin/Core/AudioSystemData.h>

#include <EditorFramework/Assets/SimpleAssetDocument.h>

class ezAmplitudeAudioControlCollectionAssetEntry : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetEntry, ezReflectedClass);

public:
  ezString m_sName;
  ezEnum<ezAmplitudeAudioControlType> m_Type;
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

class ezAmplitudeAudioControlCollectionAssetSoundBankEntry : public ezAmplitudeAudioControlCollectionAssetEntry
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetSoundBankEntry, ezAmplitudeAudioControlCollectionAssetEntry);

public:
  ezAmplitudeAudioControlCollectionAssetSoundBankEntry();
};

class ezAmplitudeAudioControlCollectionAsset : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAsset, ezReflectedClass);

public:
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetTriggerEntry> m_TriggerEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetRtpcEntry> m_RtpcEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetSwitchEntry> m_SwitchEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetEnvironmentEntry> m_EnvironmentEntries;
  ezDynamicArray<ezAmplitudeAudioControlCollectionAssetSoundBankEntry> m_SoundBankEntries;
};

class ezAmplitudeAudioControlCollectionAssetDocument : public ezSimpleAssetDocument<ezAmplitudeAudioControlCollectionAsset>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAmplitudeAudioControlCollectionAssetDocument, ezSimpleAssetDocument<ezAmplitudeAudioControlCollectionAsset>);

public:
  ezAmplitudeAudioControlCollectionAssetDocument(ezStringView szDocumentPath);

protected:
  void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
  ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, ezStringView szOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;

private:
  ezResult TransformAssetEntry(const ezAmplitudeAudioControlCollectionAssetEntry& entry, ezAmplitudeAudioControlCollectionResourceDescriptor& descriptor) const;
};
