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

#pragma once

#include <Core/World/Component.h>
#include <Core/World/World.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

/// \brief Base class for all Amplitude components, such that they all have a common ancestor
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeComponent : public ezComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezAmplitudeComponent, ezComponent);

  // ezAmplitudeComponent

private:
  // Dummy method to hide this component in the editor UI.
  virtual void ezAmplitudeComponentIsAbstract() = 0;
};
