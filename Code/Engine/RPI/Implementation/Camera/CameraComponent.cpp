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

#include <RPI/RPIPCH.h>

#include <RPI/Camera/CameraComponent.h>
#include <RPI/Core/RenderGroup.h>
#include <RPI/Core/RenderSystem.h>
#include <RPI/Core/RenderView.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

namespace RPI
{
#pragma region spCameraComponentManager

  spCameraComponentManager::spCameraComponentManager(ezWorld* pWorld)
    : ezComponentManager(pWorld)
  {
    m_pSceneContext = spRenderSystem::GetSingleton()->GetSceneContextFromWorld(pWorld);
  }

  spCameraComponentManager::~spCameraComponentManager()
  {
    m_pSceneContext = nullptr;
  }

  void spCameraComponentManager::Initialize()
  {
    SUPER::Initialize();

    spSceneContext::GetCollectEvent().AddEventHandler(ezMakeDelegate(&spCameraComponentManager::OnCollectEvent, this));
    spSceneContext::GetExtractEvent().AddEventHandler(ezMakeDelegate(&spCameraComponentManager::OnExtractEvent, this));
  }

  void spCameraComponentManager::Deinitialize()
  {
    spSceneContext::GetCollectEvent().RemoveEventHandler(ezMakeDelegate(&spCameraComponentManager::OnCollectEvent, this));
    spSceneContext::GetExtractEvent().RemoveEventHandler(ezMakeDelegate(&spCameraComponentManager::OnExtractEvent, this));

    SUPER::Deinitialize();
  }

  void spCameraComponentManager::OnCollectEvent(const spSceneContextCollectEvent& event)
  {
    EZ_LOCK(GetWorld()->GetWriteMarker());

    for (auto it = GetComponents(); it.IsValid(); ++it)
      if (it->IsActiveAndInitialized())
        it->OnCollectEvent(event);
  }

  void spCameraComponentManager::OnExtractEvent(const spSceneContextExtractEvent& event)
  {
    EZ_LOCK(GetWorld()->GetWriteMarker());

    for (auto it = GetComponents(); it.IsValid(); ++it)
      if (it->IsActiveAndInitialized())
        it->OnExtractEvent(event);
  }

#pragma endregion

#pragma region spCameraComponent

  constexpr ezTypeVersion kCameraComponentVersion = 1;

  // clang-format off
  EZ_BEGIN_COMPONENT_TYPE(spCameraComponent, kCameraComponentVersion, ezComponentMode::Static)
  {
    EZ_BEGIN_ATTRIBUTES
    {
      new ezCategoryAttribute("Rendering"),
      new ezDirectionVisualizerAttribute(ezBasisAxis::PositiveX, 1.0f, ezColor::DarkSlateBlue),
      new ezCameraVisualizerAttribute("ProjectionMode", "FieldOfView", "OrthographicSize", "NearPlaneDistance", "FarPlaneDistance"),
    }
    EZ_END_ATTRIBUTES;

    EZ_BEGIN_PROPERTIES
    {
      EZ_ACCESSOR_PROPERTY("Slot", GetCameraSlot, SetCameraSlot)->AddAttributes(new ezDynamicStringEnumAttribute("CameraSlot")),

      EZ_BITFLAGS_ACCESSOR_PROPERTY("Usage", spRenderViewUsage, GetRenderViewUsage, SetRenderViewUsage)->AddAttributes(new ezDefaultValueAttribute(spRenderViewUsage::Default)),
      EZ_BITFLAGS_ACCESSOR_PROPERTY("RenderGroup", spRenderGroup, GetRenderGroupMask, SetRenderGroupMask)->AddAttributes(new ezDefaultValueAttribute(spRenderGroup::Default)),
      EZ_ACCESSOR_PROPERTY("CullingEnabled", IsCullingEnabled, SetCullingEnabled)->AddAttributes(new ezDefaultValueAttribute(true)),

      EZ_ENUM_ACCESSOR_PROPERTY("ProjectionMode", spCameraProjectionMode, GetProjectionMode, SetProjectionMode)->AddAttributes(new ezDefaultValueAttribute(spCameraProjectionMode::Default)),
      EZ_ACCESSOR_PROPERTY("NearPlaneDistance", GetNearPlaneDistance, SetNearPlaneDistance)->AddAttributes(new ezDefaultValueAttribute(0.25f)),
      EZ_ACCESSOR_PROPERTY("FarPlaneDistance", GetFarPlaneDistance, SetFarPlaneDistance)->AddAttributes(new ezDefaultValueAttribute(3000.0f)),
      EZ_ACCESSOR_PROPERTY("FieldOfView", GetFieldOfView, SetFieldOfView)->AddAttributes(new ezDefaultValueAttribute(60.0f), new ezClampValueAttribute(1.0f, 170.0f), new ezSuffixAttribute(" deg")),
      EZ_ACCESSOR_PROPERTY("OrthographicSize", GetOrthographicSize, SetOrthographicSize)->AddAttributes(new ezDefaultValueAttribute(720.0f), new ezClampValueAttribute(0.01f, {})),
      EZ_ACCESSOR_PROPERTY("AspectRatio", GetAspectRatio, SetAspectRatio)->AddAttributes(new ezDefaultValueAttribute(16.0f / 9.0f)),
      EZ_ACCESSOR_PROPERTY("Aperture", GetAperture, SetAperture)->AddAttributes(new ezDefaultValueAttribute(2.8f ), new ezClampValueAttribute(1.0f, 32.0f), new ezSuffixAttribute(" f-stop(s)")),
      EZ_ACCESSOR_PROPERTY("ShutterSpeed", GetShutterSpeed, SetShutterSpeed)->AddAttributes(new ezDefaultValueAttribute(ezTime::MakeFromSeconds(1.0 / 60.0)), new ezSuffixAttribute(" sec")),
      EZ_ACCESSOR_PROPERTY("ISOSensitivity", GetISOSensitivity, SetISOSensitivity)->AddAttributes(new ezDefaultValueAttribute(100.0f)),
      EZ_ACCESSOR_PROPERTY("ExposureCompensation", GetExposureCompensation, SetExposureCompensation)->AddAttributes(new ezDefaultValueAttribute(0.0f)),

      EZ_ACCESSOR_PROPERTY_READ_ONLY("EV100", GetEV100),
      EZ_ACCESSOR_PROPERTY_READ_ONLY("Exposure", GetExposure),
    }
    EZ_END_PROPERTIES;
  }
  EZ_END_COMPONENT_TYPE;
  // clang-format on

  void spCameraComponent::SerializeComponent(ezWorldWriter& inout_stream) const
  {
    SUPER::SerializeComponent(inout_stream);
    auto& s = inout_stream.GetStream();

    s.WriteVersion(kCameraComponentVersion);

    m_Camera.Write(s);
  }

  void spCameraComponent::DeserializeComponent(ezWorldReader& inout_stream)
  {
    SUPER::DeserializeComponent(inout_stream);
    auto& s = inout_stream.GetStream();

    s.ReadVersion(kCameraComponentVersion);

    m_Camera.Read(s);
  }

  void spCameraComponent::OnActivated()
  {
    SUPER::OnActivated();
  }

  void spCameraComponent::OnDeactivated()
  {
    SUPER::OnDeactivated();
  }

  spCameraComponent::spCameraComponent() = default;
  spCameraComponent::~spCameraComponent() = default;

  void spCameraComponent::SetRenderViewUsage(ezBitflags<RPI::spRenderViewUsage> eUsage)
  {
    m_Camera.SetRenderViewUsage(eUsage);
  }

  ezBitflags<RPI::spRenderViewUsage> spCameraComponent::GetRenderViewUsage() const
  {
    return m_Camera.GetRenderViewUsage();
  }

  void spCameraComponent::SetProjectionMode(ezEnum<RPI::spCameraProjectionMode> eProjectionMode)
  {
    m_Camera.SetProjectionMode(eProjectionMode);
  }

  ezEnum<RPI::spCameraProjectionMode> spCameraComponent::GetProjectionMode() const
  {
    return m_Camera.GetProjectionMode();
  }

  void spCameraComponent::SetCullingEnabled(bool bIsCullingEnabled)
  {
    m_Camera.SetCullingEnabled(bIsCullingEnabled);
  }

  bool spCameraComponent::IsCullingEnabled() const
  {
    return m_Camera.IsCullingEnabled();
  }

  void spCameraComponent::SetNearPlaneDistance(float fNearPlaneDistance)
  {
    m_Camera.SetNearPlaneDistance(fNearPlaneDistance);
  }

  float spCameraComponent::GetNearPlaneDistance() const
  {
    return m_Camera.GetNearPlaneDistance();
  }

  void spCameraComponent::SetFarPlaneDistance(float fNearPlaneDistance)
  {
    m_Camera.SetFarPlaneDistance(fNearPlaneDistance);
  }

  float spCameraComponent::GetFarPlaneDistance() const
  {
    return m_Camera.GetFarPlaneDistance();
  }

  void spCameraComponent::SetFieldOfView(float fFieldOfView)
  {
    m_Camera.SetFieldOfView(fFieldOfView);
  }

  float spCameraComponent::GetFieldOfView() const
  {
    return m_Camera.GetFieldOfView();
  }

  void spCameraComponent::SetOrthographicSize(float fOrthographicSize)
  {
    m_Camera.SetOrthographicSize(fOrthographicSize);
  }

  float spCameraComponent::GetOrthographicSize() const
  {
    return m_Camera.GetOrthographicSize();
  }

  void spCameraComponent::SetAspectRatio(float fAspectRatio)
  {
    m_Camera.SetAspectRatio(fAspectRatio);
  }

  float spCameraComponent::GetAspectRatio() const
  {
    return m_Camera.GetAspectRatio();
  }

  void spCameraComponent::SetAperture(float fAperture)
  {
    m_Camera.SetAperture(fAperture);
  }

  float spCameraComponent::GetAperture() const
  {
    return m_Camera.GetAperture();
  }

  void spCameraComponent::SetShutterSpeed(ezTime fShutterSpeed)
  {
    m_Camera.SetShutterSpeed(fShutterSpeed);
  }

  ezTime spCameraComponent::GetShutterSpeed() const
  {
    return m_Camera.GetShutterSpeed();
  }

  void spCameraComponent::SetISOSensitivity(float fISOSensitivity)
  {
    m_Camera.SetISOSensitivity(fISOSensitivity);
  }

  float spCameraComponent::GetISOSensitivity() const
  {
    return m_Camera.GetISOSensitivity();
  }

  void spCameraComponent::SetExposureCompensation(float fExposureCompensation)
  {
    m_Camera.SetExposureCompensation(fExposureCompensation);
  }

  float spCameraComponent::GetExposureCompensation() const
  {
    return m_Camera.GetExposureCompensation();
  }

  void spCameraComponent::SetRenderGroupMask(ezBitflags<spRenderGroup> eRenderGroupMask)
  {
    m_Camera.SetRenderGroupMask(eRenderGroupMask);
  }

  ezBitflags<spRenderGroup> spCameraComponent::GetRenderGroupMask() const
  {
    return m_Camera.GetRenderGroupMask();
  }

  float spCameraComponent::GetEV100() const
  {
    return m_Camera.GetEV100();
  }

  float spCameraComponent::GetExposure() const
  {
    return m_Camera.GetExposure();
  }

  ezAngle spCameraComponent::GetFieldOfViewX() const
  {
    return m_Camera.GetFieldOfViewX();
  }

  ezAngle spCameraComponent::GetFieldOfViewY() const
  {
    return m_Camera.GetFieldOfViewY();
  }

  float spCameraComponent::GetOrthographicSizeX() const
  {
    return m_Camera.GetOrthographicSizeX();
  }

  float spCameraComponent::GetOrthographicSizeY() const
  {
    return m_Camera.GetOrthographicSizeY();
  }

  void spCameraComponent::OnCollectEvent(const spSceneContextCollectEvent& event)
  {
    if (event.m_Type != spSceneContextCollectEvent::Type::Collect)
      return;

    event.m_pSceneContext->GetRenderViewCollector().Add(m_Camera.GetRenderView());
  }

  void spCameraComponent::OnExtractEvent(const spSceneContextExtractEvent& event)
  {
  }

  void spCameraComponent::SetCameraSlot(const char* szCameraSlot)
  {
    if (m_sCameraSlotName == szCameraSlot)
      return;

    spCompositor* pCompositor = spRenderSystem::GetSingleton()->GetCompositor();
    pCompositor->AssignSlotToCamera(pCompositor->GetCameraSlotByName(szCameraSlot), &m_Camera);
    m_sCameraSlotName.Assign(szCameraSlot);
  }

  const char* spCameraComponent::GetCameraSlot() const
  {
    return m_sCameraSlotName;
  }

#pragma endregion
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Camera_CameraComponent);
