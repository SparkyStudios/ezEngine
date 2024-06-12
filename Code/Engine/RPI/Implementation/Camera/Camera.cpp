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

#include <RPI/Camera/Camera.h>

#include <Foundation/Utilities/GraphicsUtils.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(RPI::spCameraProjectionMode, 1)
  EZ_ENUM_CONSTANT(RPI::spCameraProjectionMode::PerspectiveFixedFOVY),
  EZ_ENUM_CONSTANT(RPI::spCameraProjectionMode::PerspectiveFixedFOVX),
  EZ_ENUM_CONSTANT(RPI::spCameraProjectionMode::OrthographicFixedHeight),
  EZ_ENUM_CONSTANT(RPI::spCameraProjectionMode::OrthographicFixedWidth),
EZ_END_STATIC_REFLECTED_ENUM;
// clang-format on

namespace RPI
{
  class spDefaultCameraCoordinateSystemProvider final : public ezCoordinateSystemProvider
  {
  public:
    spDefaultCameraCoordinateSystemProvider()
      : ezCoordinateSystemProvider(nullptr)
    {
    }

    void GetCoordinateSystem(const ezVec3& vGlobalPosition, ezCoordinateSystem& out_coordinateSystem) const override
    {
      out_coordinateSystem.m_vForwardDir = ezBasisAxis::GetBasisVector(m_ForwardAxis);
      out_coordinateSystem.m_vRightDir = ezBasisAxis::GetBasisVector(m_RightAxis);
      out_coordinateSystem.m_vUpDir = ezBasisAxis::GetBasisVector(m_UpAxis);
    }

    ezBasisAxis::Enum m_ForwardAxis = ezBasisAxis::PositiveX;
    ezBasisAxis::Enum m_RightAxis = ezBasisAxis::PositiveY;
    ezBasisAxis::Enum m_UpAxis = ezBasisAxis::PositiveZ;
  };

  spCamera::spCamera()
  {
    m_CachedViewMatrix.SetIdentity();
    m_CachedProjectionMatrix.SetIdentity();
    m_CachedInverseViewMatrix.SetIdentity();
    m_CachedInverseProjectionMatrix.SetIdentity();
    m_CachedViewProjectionMatrix.SetIdentity();
    m_CachedInverseViewProjectionMatrix.SetIdentity();

    SetCoordinateSystem(ezBasisAxis::PositiveX, ezBasisAxis::PositiveY, ezBasisAxis::PositiveZ);
  }

  spCamera::~spCamera()
  {
    if (m_pRenderView != nullptr)
      EZ_DELETE(RHI::spDeviceAllocatorWrapper::GetAllocator(), m_pRenderView);
  }

  void spCamera::SetCoordinateSystem(ezBasisAxis::Enum forwardAxis, ezBasisAxis::Enum rightAxis, ezBasisAxis::Enum axis)
  {
    auto provider = EZ_DEFAULT_NEW(spDefaultCameraCoordinateSystemProvider);
    provider->m_ForwardAxis = forwardAxis;
    provider->m_RightAxis = rightAxis;
    provider->m_UpAxis = axis;

    m_pCoordinateSystemProvider = provider;
  }

  void spCamera::SetRenderViewUsage(ezBitflags<spRenderViewUsage> eUsage)
  {
    if (m_eRenderViewUsage == eUsage)
      return;

    m_eRenderViewUsage = eUsage;
    MarkAsDirty();
  }

  void spCamera::SetProjectionMode(ezEnum<spCameraProjectionMode> eProjectionMode)
  {
    if (m_eProjectionMode == eProjectionMode)
      return;

    m_eProjectionMode = eProjectionMode;
    MarkAsDirty();
  }

  void spCamera::SetCullingEnabled(bool bIsCullingEnabled)
  {
    if (m_bCullingEnabled == bIsCullingEnabled)
      return;

    m_bCullingEnabled = bIsCullingEnabled;
    MarkAsDirty();
  }

  void spCamera::SetNearPlaneDistance(float fNearPlaneDistance)
  {
    if (m_fNearPlaneDistance == fNearPlaneDistance)
      return;

    m_fNearPlaneDistance = fNearPlaneDistance;
    MarkAsDirty();
  }

  void spCamera::SetFarPlaneDistance(float fNearPlaneDistance)
  {
    if (m_fFarPlaneDistance == fNearPlaneDistance)
      return;

    m_fFarPlaneDistance = fNearPlaneDistance;
    MarkAsDirty();
  }

  void spCamera::SetFieldOfView(float fFieldOfView)
  {
    if (m_fFieldOfView == fFieldOfView)
      return;

    m_fFieldOfView = fFieldOfView;
    MarkAsDirty();
  }

  void spCamera::SetOrthographicSize(float fOrthographicSize)
  {
    if (m_fOrthographicSize == fOrthographicSize)
      return;

    m_fOrthographicSize = fOrthographicSize;
    MarkAsDirty();
  }

  void spCamera::SetAspectRatio(float fAspectRatio)
  {
    if (m_fAspectRatio == fAspectRatio)
      return;

    m_fAspectRatio = fAspectRatio;
    MarkAsDirty();
  }

  void spCamera::SetAperture(float fAperture)
  {
    if (m_fAperture == fAperture)
      return;

    m_fAperture = fAperture;
    MarkAsDirty();
  }

  void spCamera::SetShutterSpeed(ezTime fShutterSpeed)
  {
    if (m_fShutterSpeed == fShutterSpeed)
      return;

    m_fShutterSpeed = fShutterSpeed;
    MarkAsDirty();
  }

  void spCamera::SetISOSensitivity(float fISOSensitivity)
  {
    if (m_fISOSensitivity == fISOSensitivity)
      return;

    m_fISOSensitivity = fISOSensitivity;
    MarkAsDirty();
  }

  void spCamera::SetExposureCompensation(float fExposureCompensation)
  {
    if (m_fExposureCompensation == fExposureCompensation)
      return;

    m_fExposureCompensation = fExposureCompensation;
    MarkAsDirty();
  }

  void spCamera::SetRenderGroupMask(ezBitflags<spRenderGroup> eRenderGroupMask)
  {
    if (m_eRenderGroupMask == eRenderGroupMask)
      return;

    m_eRenderGroupMask = eRenderGroupMask;
    MarkAsDirty();
  }

  float spCamera::GetEV100() const
  {
    return ezMath::Log2(m_fAperture / m_fShutterSpeed.AsFloatInSeconds() * 100.0f / m_fISOSensitivity) - m_fExposureCompensation;
  }

  float spCamera::GetExposure() const
  {
    return 1.0f / (ezMath::Pow2(GetEV100()));
  }

  ezAngle spCamera::GetFieldOfViewX() const
  {
    if (m_eProjectionMode == spCameraProjectionMode::PerspectiveFixedFOVX)
      return ezAngle::MakeFromDegree(m_fFieldOfView);

    if (m_eProjectionMode == spCameraProjectionMode::PerspectiveFixedFOVY)
      return ezMath::ATan(ezMath::Tan(ezAngle::MakeFromDegree(m_fFieldOfView) * 0.5f) * m_fAspectRatio) * 2.0f;

    EZ_REPORT_FAILURE("You cannot get the camera FOV X of an orthographic camera.");
    return ezAngle();
  }

  ezAngle spCamera::GetFieldOfViewY() const
  {
    if (m_eProjectionMode == spCameraProjectionMode::PerspectiveFixedFOVX)
      return ezMath::ATan(ezMath::Tan(ezAngle::MakeFromDegree(m_fFieldOfView) * 0.5f) / m_fAspectRatio) * 2.0f;

    if (m_eProjectionMode == spCameraProjectionMode::PerspectiveFixedFOVY)
      return ezAngle::MakeFromDegree(m_fFieldOfView);

    EZ_REPORT_FAILURE("You cannot get the camera FOV Y of an orthographic camera.");
    return ezAngle();
  }

  float spCamera::GetOrthographicSizeX() const
  {
    if (m_eProjectionMode == spCameraProjectionMode::OrthographicFixedWidth)
      return m_fOrthographicSize;

    if (m_eProjectionMode == spCameraProjectionMode::OrthographicFixedHeight)
      return m_fOrthographicSize * m_fAspectRatio;

    EZ_REPORT_FAILURE("You cannot get the camera orthographic size X of a perspective camera.");
    return 0.0f;
  }

  float spCamera::GetOrthographicSizeY() const
  {
    if (m_eProjectionMode == spCameraProjectionMode::OrthographicFixedWidth)
      return m_fOrthographicSize / m_fAspectRatio;

    if (m_eProjectionMode == spCameraProjectionMode::OrthographicFixedHeight)
      return m_fOrthographicSize;

    EZ_REPORT_FAILURE("You cannot get the camera orthographic size Y of a perspective camera.");
    return 0.0f;
  }

  void spCamera::SetPosition(const ezVec3& vPosition)
  {
    const ezVec3 vPositionIn = MapExternalToInternal(vPosition);

    if (m_vPosition == vPositionIn)
      return;

    m_vPreviousPosition = m_vPosition;
    m_vPosition = vPositionIn;
    MarkAsDirty();
  }

  void spCamera::SetForward(const ezVec3& vForward)
  {
    const ezVec3 vForwardIn = MapExternalToInternal(vForward.GetNormalized());

    if (m_vForward == vForwardIn)
      return;

    m_vForward = vForwardIn;
    MarkAsDirty();
  }

  void spCamera::SetUp(const ezVec3& vUp)
  {
    const ezVec3 vUpIn = MapExternalToInternal(vUp.GetNormalized());

    if (m_vUp == vUpIn)
      return;

    m_vUp = vUpIn;
    MarkAsDirty();
  }

  void spCamera::LookAt(const ezVec3& vCameraPos, const ezVec3& vTargetPos, const ezVec3& vUp)
  {
    const ezVec3 vCameraPosIn = MapExternalToInternal(vCameraPos);
    const ezVec3 vTargetPosIn = MapExternalToInternal(vTargetPos);
    const ezVec3 vUpIn = MapExternalToInternal(vUp.GetNormalized());

    m_vPosition = vCameraPosIn;
    m_vUp = vUpIn;
    m_vForward = (vTargetPosIn - vCameraPosIn).GetNormalized();

    MarkAsDirty();
  }

  spRenderView* spCamera::GetRenderView()
  {
    if (m_pRenderView == nullptr)
      m_pRenderView = EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spRenderView);

    if (m_bIsDirty)
    {
      UpdateView();
      m_bIsDirty = false;
    }

    return m_pRenderView;
  }

  void spCamera::GetBoundingFrustum(ezFrustum& out_frustum)
  {
  }

  void spCamera::Write(ezStreamWriter& stream) const
  {
    stream << m_eRenderViewUsage;
    stream << m_eRenderGroupMask;
    stream << m_bCullingEnabled;
    stream << m_eProjectionMode;
    stream << m_fNearPlaneDistance;
    stream << m_fFarPlaneDistance;
    stream << m_fFieldOfView;
    stream << m_fOrthographicSize;
    stream << m_fAspectRatio;
    stream << m_fAperture;
    stream << m_fShutterSpeed;
    stream << m_fISOSensitivity;
    stream << m_fExposureCompensation;
  }

  void spCamera::Read(ezStreamReader& stream)
  {
    stream >> m_eRenderViewUsage;
    stream >> m_eRenderGroupMask;
    stream >> m_bCullingEnabled;
    stream >> m_eProjectionMode;
    stream >> m_fNearPlaneDistance;
    stream >> m_fFarPlaneDistance;
    stream >> m_fFieldOfView;
    stream >> m_fOrthographicSize;
    stream >> m_fAspectRatio;
    stream >> m_fAperture;
    stream >> m_fShutterSpeed;
    stream >> m_fISOSensitivity;
    stream >> m_fExposureCompensation;

    MarkAsDirty();
  }

  void spCamera::MarkAsDirty()
  {
    m_bIsDirty = true;

    EZ_ASSERT_DEV(m_fNearPlaneDistance < m_fFarPlaneDistance, "Near plane distance must be smaller than far plane distance.");
    EZ_ASSERT_DEV(m_fFieldOfView > 0.0f, "Field of view must be greater than 0.");
    EZ_ASSERT_DEV(m_fOrthographicSize > 0.0f, "Orthographic size must be greater than 0.");
  }

  void spCamera::CacheViewMatrix()
  {
    if (!m_bIsDirty)
      return;

    m_CachedViewMatrix = ezGraphicsUtils::CreateLookAtViewMatrix(m_vPosition, m_vPosition + m_vForward, m_vUp, ezHandedness::LeftHanded);
  }

  void spCamera::CacheProjectionMatrix()
  {
    if (!m_bIsDirty)
      return;

    const RHI::spDeviceCapabilities& capabilities = spRenderSystem::GetSingleton()->GetDevice()->GetCapabilities();
    const ezClipSpaceDepthRange::Enum depthRange = capabilities.m_bIsDepthRangeZeroToOne ? ezClipSpaceDepthRange::ZeroToOne : ezClipSpaceDepthRange::MinusOneToOne;
    const ezClipSpaceYMode::Enum clipSpaceYMode = capabilities.m_bIsClipSpaceYInverted ? ezClipSpaceYMode::Flipped : ezClipSpaceYMode::Regular;

    switch (m_eProjectionMode.GetValue())
    {
      case spCameraProjectionMode::PerspectiveFixedFOVY:
        m_CachedProjectionMatrix = ezGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
          ezAngle::MakeFromDegree(m_fFieldOfView), m_fAspectRatio, m_fNearPlaneDistance, m_fFarPlaneDistance, depthRange, clipSpaceYMode, ezHandedness::LeftHanded);
        break;

      case spCameraProjectionMode::PerspectiveFixedFOVX:
        m_CachedProjectionMatrix = ezGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(
          ezAngle::MakeFromDegree(m_fFieldOfView), m_fAspectRatio, m_fNearPlaneDistance, m_fFarPlaneDistance, depthRange, clipSpaceYMode, ezHandedness::LeftHanded);
        break;

      case spCameraProjectionMode::OrthographicFixedWidth:
      case spCameraProjectionMode::OrthographicFixedHeight:
        m_CachedProjectionMatrix = ezGraphicsUtils::CreateOrthographicProjectionMatrix(
          GetOrthographicSizeX(), GetOrthographicSizeY(), m_fNearPlaneDistance, m_fFarPlaneDistance, depthRange, clipSpaceYMode, ezHandedness::LeftHanded);
        break;

      default:
        EZ_REPORT_FAILURE("Invalid Camera Mode {0}", static_cast<int>(m_eProjectionMode));
    }
  }

  void spCamera::CacheViewProjectionMatrix()
  {
    if (!m_bIsDirty)
      return;

    m_CachedViewProjectionMatrix = m_CachedProjectionMatrix * m_CachedViewMatrix;
  }


  void spCamera::UpdateView()
  {
    if (m_pRenderView == nullptr)
      return;

    const ezMat4 previousViewProjectionMatrix = m_CachedViewProjectionMatrix;
    const ezMat4 previousInverseViewProjectionMatrix = m_CachedViewProjectionMatrix.GetInverse(0.0f);

    CacheViewMatrix();
    CacheProjectionMatrix();
    CacheViewProjectionMatrix();

    m_pRenderView->SetRenderGroup(m_eRenderGroupMask);
    m_pRenderView->SetCullingMode(m_bCullingEnabled ? spRenderViewCullingMode::Frustum : spRenderViewCullingMode::None);
    m_pRenderView->SetUsage(m_eRenderViewUsage);

    m_pRenderView->SetViewMatrix(m_CachedViewMatrix);
    m_pRenderView->SetProjectionMatrix(m_CachedProjectionMatrix);

    {
      const auto viewData = m_pRenderView->GetDataBuffer().Write();
      viewData->m_Position = GetPosition();
      viewData->m_NearClipPlane = GetNearPlaneDistance();
      viewData->m_PreviousPosition = m_vPreviousPosition;
      viewData->m_FarClipPlane = GetFarPlaneDistance();
      viewData->m_Direction = GetForward();
      viewData->m_ShutterSpeed = GetShutterSpeed().AsFloatInSeconds();
      viewData->m_Exposure = GetExposure();
      viewData->m_Aperture = GetAperture();
      viewData->m_ISO = GetISOSensitivity();
      viewData->m_AspectRatio = GetAspectRatio();
      viewData->m_Projection = m_CachedProjectionMatrix;
      viewData->m_InverseProjection = m_CachedProjectionMatrix.GetInverse(0.0f);
      viewData->m_View = m_CachedViewMatrix;
      viewData->m_InverseView = m_CachedViewMatrix.GetInverse(0.0f);
      viewData->m_ViewProjection = m_CachedViewProjectionMatrix;
      viewData->m_InverseViewProjection = m_CachedViewProjectionMatrix.GetInverse(0.0f);
      viewData->m_PreviousViewProjection = previousViewProjectionMatrix;
      viewData->m_PreviousInverseViewProjection = previousInverseViewProjectionMatrix;
    }
  }

  ezVec3 spCamera::MapExternalToInternal(const ezVec3& v) const
  {
    if (m_pCoordinateSystemProvider == nullptr)
      return v;

    ezCoordinateSystem system;
    m_pCoordinateSystemProvider->GetCoordinateSystem(m_vPosition, system);

    ezMat3 m;
    m.SetRow(0, system.m_vForwardDir);
    m.SetRow(1, system.m_vRightDir);
    m.SetRow(2, system.m_vUpDir);

    return m * v;
  }

  ezVec3 spCamera::MapInternalToExternal(const ezVec3& v) const
  {
    if (m_pCoordinateSystemProvider == nullptr)
      return v;

    ezCoordinateSystem system;
    m_pCoordinateSystemProvider->GetCoordinateSystem(m_vPosition, system);

    ezMat3 m;
    m.SetColumn(0, system.m_vForwardDir);
    m.SetColumn(1, system.m_vRightDir);
    m.SetColumn(2, system.m_vUpDir);

    return m * v;
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Camera_Camera);
