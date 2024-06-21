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

#include <RPI/RPIDLL.h>

#include <Core/World/World.h>

#include <RPI/Camera/Camera.h>
#include <RPI/Scene/SceneContext.h>

namespace RPI
{
  class spRenderView;

  class SP_RPI_DLL spCameraComponentManager : public ezComponentManager<class spCameraComponent, ezBlockStorageType::Compact>
  {
    friend class spCameraComponent;

  public:
    using SUPER = ezComponentManager<class spCameraComponent, ezBlockStorageType::Compact>;

    explicit spCameraComponentManager(ezWorld* pWorld);
    ~spCameraComponentManager() override;

    void Initialize() override;
    void Deinitialize() override;

    void Update(const ezWorldModule::UpdateContext& context);

  private:
    void OnCollectEvent(const spSceneContextCollectEvent& event);
    void OnExtractEvent(const spSceneContextExtractEvent& event);

    spSceneContext* m_pSceneContext{nullptr};
  };

  class SP_RPI_DLL spCameraComponent : public ezComponent
  {
    friend class spCameraComponentManager;

    EZ_DECLARE_COMPONENT_TYPE(spCameraComponent, ezComponent, spCameraComponentManager);

    // ezComponent

  public:
    void SerializeComponent(ezWorldWriter& inout_stream) const override;
    void DeserializeComponent(ezWorldReader& inout_stream) override;

  protected:
    void OnActivated() override;
    void OnDeactivated() override;

    // spCameraComponent

  public:
    spCameraComponent();
    ~spCameraComponent() override;

#pragma region Properties

    void SetRenderViewUsage(ezBitflags<spRenderViewUsage> eUsage);
    [[nodiscard]] ezBitflags<spRenderViewUsage> GetRenderViewUsage() const;

    void SetProjectionMode(ezEnum<spCameraProjectionMode> eProjectionMode);
    [[nodiscard]] ezEnum<spCameraProjectionMode> GetProjectionMode() const;

    void SetCullingEnabled(bool bIsCullingEnabled);
    [[nodiscard]] bool IsCullingEnabled() const;

    void SetNearPlaneDistance(float fNearPlaneDistance);
    [[nodiscard]] float GetNearPlaneDistance() const;

    void SetFarPlaneDistance(float fFarPlaneDistance);
    [[nodiscard]] float GetFarPlaneDistance() const;

    void SetFieldOfView(float fFieldOfView);
    [[nodiscard]] float GetFieldOfView() const;

    void SetOrthographicSize(float fOrthographicSize);
    [[nodiscard]] float GetOrthographicSize() const;

    void SetAspectRatio(float fAspectRatio);
    [[nodiscard]] float GetAspectRatio() const;

    void SetAperture(float fAperture);
    [[nodiscard]] float GetAperture() const;

    void SetShutterSpeed(ezTime fShutterSpeed);
    [[nodiscard]] ezTime GetShutterSpeed() const;

    void SetISOSensitivity(float fISOSensitivity);
    [[nodiscard]] float GetISOSensitivity() const;

    void SetExposureCompensation(float fExposureCompensation);
    [[nodiscard]] float GetExposureCompensation() const;

    void SetRenderGroupMask(ezBitflags<spRenderGroup> eRenderGroupMask);
    ezBitflags<spRenderGroup> GetRenderGroupMask() const;

    [[nodiscard]] float GetEV100() const;
    [[nodiscard]] float GetExposure() const;

    [[nodiscard]] ezAngle GetFieldOfViewX() const;
    [[nodiscard]] ezAngle GetFieldOfViewY() const;

    [[nodiscard]] float GetOrthographicSizeX() const;
    [[nodiscard]] float GetOrthographicSizeY() const;

    void SetCameraSlot(const char* szCameraSlot);
    [[nodiscard]] const char* GetCameraSlot() const;

#pragma endregion

  private:
    void OnCollectEvent(const spSceneContextCollectEvent& event);
    void OnExtractEvent(const spSceneContextExtractEvent& event);

    spCamera m_Camera;
    ezHashedString m_sCameraSlotName;
  };
} // namespace RPI
