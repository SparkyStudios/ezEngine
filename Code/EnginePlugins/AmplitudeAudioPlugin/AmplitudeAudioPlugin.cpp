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

// BEGIN-DOCS-CODE-SNIPPET: plugin-setup
EZ_PLUGIN_ON_LOADED()
{
  // you could do something here, though this is rare
}

EZ_PLUGIN_ON_UNLOADED()
{
  // you could do something here, though this is rare
}
// END-DOCS-CODE-SNIPPET
