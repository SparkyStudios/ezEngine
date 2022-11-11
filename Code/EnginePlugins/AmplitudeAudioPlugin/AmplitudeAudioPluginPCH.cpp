#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>
#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Strings/TranslationLookup.h>

EZ_STATICLINK_LIBRARY(AmplitudeAudioPlugin)
{
  if (bReturn)
    return;

  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_AmplitudeAudioSingleton);
  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_AmplitudeAudioPluginStartup);
  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_Core_AudioMiddlewareControlsManager);

  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_Components_AudioControlsComponent);
  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_Components_AmplitudeComponent);
  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_Components_AmplitudeListenerComponent);
  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_Components_AmplitudeTriggerComponent);

  EZ_STATICLINK_REFERENCE(AmplitudeAudioPlugin_Resources_AudioControlCollectionResource);
}
