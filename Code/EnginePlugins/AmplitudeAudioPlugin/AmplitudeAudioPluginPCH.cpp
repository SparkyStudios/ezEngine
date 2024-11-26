// Copyright (c) 2022-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
