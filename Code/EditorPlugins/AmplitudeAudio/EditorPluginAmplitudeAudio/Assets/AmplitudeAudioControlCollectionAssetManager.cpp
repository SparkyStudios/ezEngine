#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAsset.h>
#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAssetManager.h>
#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAssetWindow.moc.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAmplitudeAudioControlCollectionAssetDocumentManager, 1, ezRTTIDefaultAllocator<ezAmplitudeAudioControlCollectionAssetDocumentManager>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezAmplitudeAudioControlCollectionAssetDocumentManager::ezAmplitudeAudioControlCollectionAssetDocumentManager()
{
  ezDocumentManager::s_Events.AddEventHandler(ezMakeDelegate(&ezAmplitudeAudioControlCollectionAssetDocumentManager::OnDocumentManagerEvent, this));

  m_DocTypeDesc.m_sDocumentTypeName = "Audio Control Collection";
  m_DocTypeDesc.m_sFileExtension = "ezAudioControlCollectionAsset";
  m_DocTypeDesc.m_sIcon = ":/AssetIcons/Audio_Control_Collection.png";
  m_DocTypeDesc.m_pDocumentType = ezGetStaticRTTI<ezAmplitudeAudioControlCollectionAssetDocument>();
  m_DocTypeDesc.m_pManager = this;
  m_DocTypeDesc.m_CompatibleTypes.PushBack("CompatibleAsset_AmplitudeAudio_Audio_Control_Collection");

  m_DocTypeDesc.m_sResourceFileExtension = "ezAudioSystemControls";
  m_DocTypeDesc.m_AssetDocumentFlags = ezAssetDocumentFlags::AutoTransformOnSave;

  ezQtImageCache::GetSingleton()->RegisterTypeImage("Audio Control Collection", QPixmap(":/AssetIcons/Audio_Control_Collection.png"));
}

ezAmplitudeAudioControlCollectionAssetDocumentManager::~ezAmplitudeAudioControlCollectionAssetDocumentManager()
{
  ezDocumentManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezAmplitudeAudioControlCollectionAssetDocumentManager::OnDocumentManagerEvent, this));
}

void ezAmplitudeAudioControlCollectionAssetDocumentManager::OnDocumentManagerEvent(const ezDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case ezDocumentManager::Event::Type::DocumentWindowRequested:
    {
      if (e.m_pDocument->GetDynamicRTTI() == ezGetStaticRTTI<ezAmplitudeAudioControlCollectionAssetDocument>())
      {
        auto* pDocWnd = new ezQtAmplitudeAudioControlCollectionAssetDocumentWindow(e.m_pDocument);
      }
    }
    break;

    default:
      break;
  }
}

void ezAmplitudeAudioControlCollectionAssetDocumentManager::InternalCreateDocument(
  const char* szDocumentTypeName, const char* szPath, bool bCreateNewDocument, ezDocument*& out_pDocument, const ezDocumentObject* pOpenContext)
{
  out_pDocument = new ezAmplitudeAudioControlCollectionAssetDocument(szPath);
}

void ezAmplitudeAudioControlCollectionAssetDocumentManager::InternalGetSupportedDocumentTypes(ezDynamicArray<const ezDocumentTypeDescriptor*>& inout_DocumentTypes) const
{
  inout_DocumentTypes.PushBack(&m_DocTypeDesc);
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(EditorPluginAmplitudeAudio, EditorPluginAmplitudeAudio_Assets_AmplitudeAudioControlCollectionAssetManager);
