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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeRoomComponent.h>
#include <AmplitudeAudioPlugin/Core/Common.h>
#include <AudioSystemPlugin/Core/AudioSystemRequests.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

constexpr ezTypeVersion kVersion_AmplitudeRoomComponent = 1;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezAmplitudeRoomComponent, kVersion_AmplitudeRoomComponent, ezComponentMode::Static)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezBoxManipulatorAttribute("HalfExtends", 2.0f, true),
    new ezBoxVisualizerAttribute("HalfExtends", 2.0f, ezColor::White, "Color"),
  }
  EZ_END_ATTRIBUTES;

  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Color", m_ShapeColor),
    EZ_ACCESSOR_PROPERTY("HalfExtends", GetHalfExtends, SetHalfExtends)->AddAttributes(new ezDefaultValueAttribute(ezVec3(0.5f)), new ezClampValueAttribute(0.1f, ezVariant()), new ezSuffixAttribute(" m")),
    EZ_ACCESSOR_PROPERTY_READ_ONLY("RoomID", GetRoomID),
    EZ_ACCESSOR_PROPERTY("Gain", GetGain, SetGain)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 1.0f)),

    EZ_RESOURCE_MEMBER_PROPERTY("LeftWallMaterial", m_hLeftWallMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_CustomData", "ezAmplitudeRoomMaterial")),
    EZ_RESOURCE_MEMBER_PROPERTY("RightWallMaterial", m_hRightWallMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_CustomData", "ezAmplitudeRoomMaterial")),
    EZ_RESOURCE_MEMBER_PROPERTY("FrontWallMaterial", m_hFrontWallMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_CustomData", "ezAmplitudeRoomMaterial")),
    EZ_RESOURCE_MEMBER_PROPERTY("BackWallMaterial", m_hBackWallMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_CustomData", "ezAmplitudeRoomMaterial")),
    EZ_RESOURCE_MEMBER_PROPERTY("CeilingMaterial", m_hCeilingMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_CustomData", "ezAmplitudeRoomMaterial")),
    EZ_RESOURCE_MEMBER_PROPERTY("FloorMaterial", m_hFloorMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_CustomData", "ezAmplitudeRoomMaterial")),
  }
  EZ_END_PROPERTIES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezAmplitudeRoomComponent::Initialize()
{
  SUPER::Initialize();
}

void ezAmplitudeRoomComponent::Deinitialize()
{
  SUPER::Deinitialize();
}

void ezAmplitudeRoomComponent::OnActivated()
{
  SUPER::OnActivated();

  if (amEngine->IsInitialized())
    InitializeRoom();
  else
    amEngine->OnNextFrame([this](double)
    {
      InitializeRoom();
    });
}

void ezAmplitudeRoomComponent::OnDeactivated()
{
  if (m_Room.Valid())
    amEngine->RemoveRoom(m_uiRoomID);

  ezAmplitudeComponent::OnDeactivated();
}

void ezAmplitudeRoomComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AmplitudeRoomComponent);

  s << m_uiRoomID;
  s << m_vHalfExtends;

  s << m_hLeftWallMaterial;
  s << m_hRightWallMaterial;
  s << m_hFrontWallMaterial;
  s << m_hBackWallMaterial;
  s << m_hCeilingMaterial;
  s << m_hFloorMaterial;
}

void ezAmplitudeRoomComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AmplitudeRoomComponent);

  s >> m_uiRoomID;
  s >> m_vHalfExtends;

  s >> m_hLeftWallMaterial;
  s >> m_hRightWallMaterial;
  s >> m_hFrontWallMaterial;
  s >> m_hBackWallMaterial;
  s >> m_hCeilingMaterial;
  s >> m_hFloorMaterial;
}

ezAmplitudeRoomComponent::ezAmplitudeRoomComponent()
  : m_fGain(1.0f)
{
  ezRandom r;
  r.InitializeFromCurrentTime();
  m_uiRoomID = r.UInt();
}

const ezVec3& ezAmplitudeRoomComponent::GetHalfExtends() const
{
  return m_vHalfExtends;
}

void ezAmplitudeRoomComponent::SetHalfExtends(const ezVec3& vHalfExtends)
{
  m_vHalfExtends = vHalfExtends;
}

AmRoomID ezAmplitudeRoomComponent::GetRoomID() const
{
  return m_uiRoomID;
}

AmReal32 ezAmplitudeRoomComponent::GetGain() const
{
  return m_fGain;
}

void ezAmplitudeRoomComponent::SetGain(AmReal32 fGain)
{
  m_fGain = fGain;
}

ezAmplitudeRoomMaterialResourceHandle ezAmplitudeRoomComponent::GetLeftWallMaterial() const
{
  return m_hLeftWallMaterial;
}

ezAmplitudeRoomMaterialResourceHandle ezAmplitudeRoomComponent::GetRightWallMaterial() const
{
  return m_hRightWallMaterial;
}

ezAmplitudeRoomMaterialResourceHandle ezAmplitudeRoomComponent::GetFrontWallMaterial() const
{
  return m_hFrontWallMaterial;
}

ezAmplitudeRoomMaterialResourceHandle ezAmplitudeRoomComponent::GetBackWallMaterial() const
{
  return m_hBackWallMaterial;
}

ezAmplitudeRoomMaterialResourceHandle ezAmplitudeRoomComponent::GetCeilingMaterial() const
{
  return m_hCeilingMaterial;
}

ezAmplitudeRoomMaterialResourceHandle ezAmplitudeRoomComponent::GetFloorMaterial() const
{
  return m_hFloorMaterial;
}

void ezAmplitudeRoomComponent::Update()
{
  if (!m_Room.Valid())
    return;

  const ezGameObject* pOwner = GetOwner();

  m_Room.SetDimensions(Utils::ezVec3ToAmVec3(m_vHalfExtends * 2.0f));
  m_Room.SetGain(m_fGain);
  m_Room.SetLocation(Utils::ezVec3ToAmVec3(pOwner->GetGlobalPosition()));
  m_Room.SetOrientation(Orientation(Utils::ezVec3ToAmVec3(pOwner->GetGlobalDirForwards()), Utils::ezVec3ToAmVec3(pOwner->GetGlobalDirUp())));

  const auto updateMaterial = [this](RoomWall eWall, ezAmplitudeRoomMaterialResourceHandle hMaterial)
  {
    ezResourceLock pWallMaterial(hMaterial, ezResourceAcquireMode::AllowLoadingFallback_NeverFail);
    if (pWallMaterial.GetAcquireResult() == ezResourceAcquireResult::Final)
    {
      const ezAmplitudeRoomMaterial* pMaterial = pWallMaterial->GetData();

      RoomMaterial material;
      ezMemoryUtils::Copy(material.m_absorptionCoefficients, pMaterial->m_absorptionCoefficients.GetData(), 9);

      m_Room.SetWallMaterial(eWall, material);
    }
  };

  updateMaterial(RoomWall::Left, m_hLeftWallMaterial);
  updateMaterial(RoomWall::Right, m_hRightWallMaterial);
  updateMaterial(RoomWall::Front, m_hFrontWallMaterial);
  updateMaterial(RoomWall::Back, m_hBackWallMaterial);
  updateMaterial(RoomWall::Ceiling, m_hCeilingMaterial);
  updateMaterial(RoomWall::Floor, m_hFloorMaterial);
}

void ezAmplitudeRoomComponent::InitializeRoom()
{
  m_Room = amEngine->AddRoom(m_uiRoomID);

  Update();
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AmplitudeRoomComponent);
