// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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
#include <AmplitudeAudioPlugin/Resources/AmplitudeRoomMaterialResource.h>

#include <AudioSystemPlugin/Components/AudioSystemComponent.h>

#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

typedef ezAudioSystemComponentManager<class ezAmplitudeRoomComponent> ezAmplitudeRoomComponentManager;

/// \brief Component that applies environmental effects in a box shape.
class EZ_AMPLITUDEAUDIOPLUGIN_DLL ezAmplitudeRoomComponent : public ezAmplitudeComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezAmplitudeRoomComponent, ezAmplitudeComponent, ezAmplitudeRoomComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void OnActivated() override;
  void OnDeactivated() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  // ezAmplitudeComponent

protected:
  void ezAmplitudeComponentIsAbstract() override {}

  // ezAmplitudeRoomComponent

public:
  ezAmplitudeRoomComponent();

  /// \brief Gets the radius of the sphere that
  /// specifies the environment.
  [[nodiscard]] const ezVec3& GetHalfExtends() const;

  /// \brief Sets the radius of the sphere that
  /// specifies the environment.
  void SetHalfExtends(const ezVec3& vHalfExtends);

  /// \brief Gets the room ID.
  [[nodiscard]] SparkyStudios::Audio::Amplitude::AmRoomID GetRoomID() const;

  /// \brief Gets the gain applied to the room.
  [[nodiscard]] SparkyStudios::Audio::Amplitude::AmReal32 GetGain() const;

  /// \brief Sets the gain applied to the room.
  void SetGain(SparkyStudios::Audio::Amplitude::AmReal32 fGain);

  /// \brief Gets the material resource handle for the left wall.
  [[nodiscard]] ezAmplitudeRoomMaterialResourceHandle GetLeftWallMaterial() const;

  /// \brief Gets the material resource handle for the right wall.
  [[nodiscard]] ezAmplitudeRoomMaterialResourceHandle GetRightWallMaterial() const;

  /// \brief Gets the material resource handle for the front wall.
  [[nodiscard]] ezAmplitudeRoomMaterialResourceHandle GetFrontWallMaterial() const;

  /// \brief Gets the material resource handle for the back wall.
  [[nodiscard]] ezAmplitudeRoomMaterialResourceHandle GetBackWallMaterial() const;

  /// \brief Gets the material resource handle for the ceiling.
  [[nodiscard]] ezAmplitudeRoomMaterialResourceHandle GetCeilingMaterial() const;

  /// \brief Gets the material resource handle for the floor.
  [[nodiscard]] ezAmplitudeRoomMaterialResourceHandle GetFloorMaterial() const;

protected:
  void Update();

private:
  void InitializeRoom();

  ezColor m_ShapeColor;

  ezVec3 m_vHalfExtends;
  SparkyStudios::Audio::Amplitude::AmRoomID m_uiRoomID;
  SparkyStudios::Audio::Amplitude::AmReal32 m_fGain;

  ezAmplitudeRoomMaterialResourceHandle m_hLeftWallMaterial;
  ezAmplitudeRoomMaterialResourceHandle m_hRightWallMaterial;
  ezAmplitudeRoomMaterialResourceHandle m_hFrontWallMaterial;
  ezAmplitudeRoomMaterialResourceHandle m_hBackWallMaterial;
  ezAmplitudeRoomMaterialResourceHandle m_hCeilingMaterial;
  ezAmplitudeRoomMaterialResourceHandle m_hFloorMaterial;

  SparkyStudios::Audio::Amplitude::Room m_Room;
};
