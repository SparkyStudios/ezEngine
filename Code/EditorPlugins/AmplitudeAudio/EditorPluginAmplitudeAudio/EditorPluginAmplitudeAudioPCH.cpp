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
