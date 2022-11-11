#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

EZ_STATICLINK_LIBRARY(EditorPluginAmplitudeAudio)
{
  if (bReturn)
    return;

  
  EZ_STATICLINK_REFERENCE(EditorPluginAmplitudeAudio_Assets_AmplitudeAudioControlCollectionAsset);
  EZ_STATICLINK_REFERENCE(EditorPluginAmplitudeAudio_Assets_AmplitudeAudioControlCollectionAssetManager);
  EZ_STATICLINK_REFERENCE(EditorPluginAmplitudeAudio_Assets_AmplitudeAudioControlCollectionAssetWindow);

  EZ_STATICLINK_REFERENCE(EditorPluginAmplitudeAudio_AmplitudeAudioControlsManager);
}
