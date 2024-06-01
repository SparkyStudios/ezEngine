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

#include <RPI/Camera/CameraSlot.h>
#include <RPI/Core/RenderGroup.h>
#include <RPI/Core/RenderView.h>
#include <RPI/Scene/SceneContext.h>

class ezCoordinateSystemProvider;

namespace RPI
{
  /// \brief Describes the camera projection mode.
  ///
  /// The camera projection modes are fixed over an axis (either X or Y).
  /// The value of the other axis is computed through the aspect ratio.
  struct SP_RPI_DLL spCameraProjectionMode
  {
    typedef ezUInt8 StorageType;

    enum Enum : StorageType
    {
      None = 0,
      PerspectiveFixedFOVY,
      PerspectiveFixedFOVX,
      OrthographicFixedHeight,
      OrthographicFixedWidth,

      Default = PerspectiveFixedFOVY,
    };
  };

  class SP_RPI_DLL spCamera
  {
    friend class spCameraComponent;
    friend class spCompositor;

  public:
    spCamera();
    ~spCamera();

    /// \brief Allows to specify a different coordinate system in which the camera input and output coordinates are given.
    ///
    /// The default is forward = PositiveX, right = PositiveY, Up = PositiveZ.
    void SetCoordinateSystem(ezBasisAxis::Enum forwardAxis, ezBasisAxis::Enum rightAxis, ezBasisAxis::Enum axis);

    void SetRenderViewUsage(ezBitflags<spRenderViewUsage> eUsage);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezBitflags<spRenderViewUsage> GetRenderViewUsage() const { return m_eRenderViewUsage; }

    void SetProjectionMode(ezEnum<spCameraProjectionMode> eProjectionMode);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezEnum<spCameraProjectionMode> GetProjectionMode() const { return m_eProjectionMode; }

    void SetCullingEnabled(bool bIsCullingEnabled);
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsCullingEnabled() const { return m_bCullingEnabled; }

    void SetNearPlaneDistance(float fNearPlaneDistance);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetNearPlaneDistance() const { return m_fNearPlaneDistance; }

    void SetFarPlaneDistance(float fFarPlaneDistance);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetFarPlaneDistance() const { return m_fFarPlaneDistance; }

    void SetFieldOfView(float fFieldOfView);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetFieldOfView() const { return m_fFieldOfView; }

    void SetOrthographicSize(float fOrthographicSize);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetOrthographicSize() const { return m_fOrthographicSize; }

    void SetAspectRatio(float fAspectRatio);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetAspectRatio() const { return m_fAspectRatio; }

    void SetAperture(float fAperture);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetAperture() const { return m_fAperture; }

    void SetShutterSpeed(ezTime fShutterSpeed);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezTime GetShutterSpeed() const { return m_fShutterSpeed; }

    void SetISOSensitivity(float fISOSensitivity);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetISOSensitivity() const { return m_fISOSensitivity; }

    void SetExposureCompensation(float fExposureCompensation);
    EZ_NODISCARD EZ_ALWAYS_INLINE float GetExposureCompensation() const { return m_fExposureCompensation; }

    void SetRenderGroupMask(ezBitflags<spRenderGroup> eRenderGroupMask);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezBitflags<spRenderGroup> GetRenderGroupMask() const { return m_eRenderGroupMask; }

    EZ_NODISCARD float GetEV100() const;
    EZ_NODISCARD float GetExposure() const;

    EZ_NODISCARD ezAngle GetFieldOfViewX() const;
    EZ_NODISCARD ezAngle GetFieldOfViewY() const;

    EZ_NODISCARD float GetOrthographicSizeX() const;
    EZ_NODISCARD float GetOrthographicSizeY() const;

    void SetPosition(const ezVec3& vPosition);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezVec3 GetPosition() const { return MapInternalToExternal(m_vPosition); }

    void SetForward(const ezVec3& vForward);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezVec3 GetForward() const { return MapInternalToExternal(m_vForward); }

    void SetUp(const ezVec3& vUp);
    EZ_NODISCARD EZ_ALWAYS_INLINE ezVec3 GetUp() const { return MapInternalToExternal(m_vUp); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezVec3 GetLeft() const { return GetUp().CrossRH(GetForward()); }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezVec3 GetRight() const { return GetForward().CrossRH(GetUp()); }

    /// \brief Repositions the camera such that it looks at the given target position.
    void LookAt(const ezVec3& vCameraPos, const ezVec3& vTargetPos, const ezVec3& vUp);

    spRenderView* GetRenderView();

    void GetBoundingFrustum(ezFrustum& out_frustum);

    void Write(ezStreamWriter& stream) const;
    void Read(ezStreamReader& stream);

  private:
    void MarkAsDirty();

    void CacheViewMatrix();
    void CacheProjectionMatrix();
    void CacheViewProjectionMatrix();

    void UpdateView();

    ezVec3 MapExternalToInternal(const ezVec3& v) const;
    ezVec3 MapInternalToExternal(const ezVec3& v) const;

    bool m_bIsDirty{true};

    spCameraSlotHandle m_hCameraSlot;

    // Render View Properties
    ezBitflags<spRenderViewUsage> m_eRenderViewUsage{spRenderViewUsage::Default};
    ezBitflags<spRenderGroup> m_eRenderGroupMask{spRenderGroup::Default};
    bool m_bCullingEnabled{true};

    // Camera Properties
    ezEnum<spCameraProjectionMode> m_eProjectionMode{spCameraProjectionMode::Default};
    float m_fNearPlaneDistance{0.25f};
    float m_fFarPlaneDistance{3000.0f};
    float m_fFieldOfView{60.0f};
    float m_fOrthographicSize{720.0f};
    float m_fAspectRatio{16.0f / 9.0f};
    float m_fAperture{2.8f};
    ezTime m_fShutterSpeed{ezTime::MakeFromSeconds(1.0 / 60.0)};
    float m_fISOSensitivity{500.0f};
    float m_fExposureCompensation{0.0f};

    ezVec3 m_vPreviousPosition{ezVec3::MakeZero()};
    ezVec3 m_vPosition{ezVec3::MakeZero()};
    ezVec3 m_vForward{ezVec3::MakeAxisX()};
    ezVec3 m_vUp{ezVec3::MakeAxisZ()};

    ezSharedPtr<ezCoordinateSystemProvider> m_pCoordinateSystemProvider;

    // Cached Values
    ezMat4 m_CachedViewMatrix;
    ezMat4 m_CachedInverseViewMatrix;
    ezMat4 m_CachedProjectionMatrix;
    ezMat4 m_CachedInverseProjectionMatrix;
    ezMat4 m_CachedViewProjectionMatrix;
    ezMat4 m_CachedInverseViewProjectionMatrix;

    spRenderView* m_pRenderView{nullptr};
  };
} // namespace RPI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spCameraProjectionMode);
