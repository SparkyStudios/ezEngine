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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>
#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>
#include <AudioSystemPlugin/Core/AudioSystemData.h>

typedef ezComponentManager<class ezAudioControlsComponent, ezBlockStorageType::FreeList> ezAudioControlsComponentManager;

/// \brief Component used to load and unload a set of audio controls.
///
/// The audio controls are provided by the selected audio control collection. Loaded audio
/// controls can be automatically registered to the audio system at component activation.
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAudioControlsComponent : public ezAmplitudeComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAudioControlsComponent, ezAmplitudeComponent, ezAudioControlsComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAmplitudeComponent

private:
  void ezAmplitudeComponentIsAbstract() override {}

  // ezAudioControlsComponent

public:
  ezAudioControlsComponent();
  ~ezAudioControlsComponent() override;

  /// \brief Load the audio controls from the given collection.
  /// This is automatically called on component initialization when
  /// the AutoLoad property is set to true.
  bool Load();

  /// \brief Unloads the audio controls.
  bool Unload();

private:
  ezString m_sControlsAsset;
  bool m_bAutoLoad;

  bool m_bLoaded;
  ezAudioControlCollectionResourceHandle m_hControlsResource;
};
