#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>

// clang-format off
EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(ezAmplitudeComponent, 1)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Sound/Amplitude"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_ABSTRACT_COMPONENT_TYPE;
// clang-format on

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AmplitudeComponent);
