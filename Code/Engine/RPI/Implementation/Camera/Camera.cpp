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
  spCamera::spCamera()
  {
    m_CachedViewMatrix.SetIdentity();
    m_CachedProjectionMatrix.SetIdentity();
    m_CachedInverseViewMatrix.SetIdentity();
    m_CachedInverseProjectionMatrix.SetIdentity();
  }

  spCamera::~spCamera()
  {
    if (m_pRenderView != nullptr)
      EZ_DELETE(RHI::spDeviceAllocatorWrapper::GetAllocator(), m_pRenderView);
  }

  void spCamera::SetRenderViewUsage(ezBitflags<RPI::spRenderViewUsage> eUsage)
  {
    m_eRenderViewUsage = eUsage;
    MarkAsDirty();
  }

  void spCamera::SetProjectionMode(ezEnum<RPI::spCameraProjectionMode> eProjectionMode)
  {
    m_eProjectionMode = eProjectionMode;
    MarkAsDirty();
  }

  void spCamera::SetCullingEnabled(bool bIsCullingEnabled)
  {
    m_bCullingEnabled = bIsCullingEnabled;
    MarkAsDirty();
  }

  void spCamera::SetNearPlaneDistance(float fNearPlaneDistance)
  {
    m_fNearPlaneDistance = fNearPlaneDistance;
    MarkAsDirty();
  }

  void spCamera::SetFarPlaneDistance(float fNearPlaneDistance)
  {
    m_fFarPlaneDistance = fNearPlaneDistance;
    MarkAsDirty();
  }

  void spCamera::SetFieldOfView(float fFieldOfView)
  {
    m_fFieldOfView = fFieldOfView;
    MarkAsDirty();
  }

  void spCamera::SetOrthographicSize(float fOrthographicSize)
  {
    m_fOrthographicSize = fOrthographicSize;
    MarkAsDirty();
  }

  void spCamera::SetAspectRatio(float fAspectRatio)
  {
    m_fAspectRatio = fAspectRatio;
    MarkAsDirty();
  }

  void spCamera::SetAperture(float fAperture)
  {
    m_fAperture = fAperture;
    MarkAsDirty();
  }

  void spCamera::SetShutterSpeed(ezTime fShutterSpeed)
  {
    m_fShutterSpeed = fShutterSpeed;
    MarkAsDirty();
  }

  void spCamera::SetISOSensitivity(float fISOSensitivity)
  {
    m_fISOSensitivity = fISOSensitivity;
    MarkAsDirty();
  }

  void spCamera::SetExposureCompensation(float fExposureCompensation)
  {
    m_fExposureCompensation = fExposureCompensation;
    MarkAsDirty();
  }

  void spCamera::SetRenderGroupMask(ezBitflags<spRenderGroupMask> eRenderGroupMask)
  {
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
    m_vPreviousPosition = m_vPosition;
    m_vPosition = vPosition;
    MarkAsDirty();
  }

  void spCamera::SetForward(const ezVec3& vForward)
  {
    m_vForward = -(vForward.GetNormalized());
    MarkAsDirty();
  }

  void spCamera::SetUp(const ezVec3& vUp)
  {
    m_vUp = vUp;
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

  void spCamera::CacheProjectionMatrix()
  {
    if (!m_bIsDirty)
      return;

    const RHI::spDeviceCapabilities& capabilities = spRenderSystem::GetSingleton()->GetDevice()->GetCapabilities();
    ezClipSpaceDepthRange::Enum depthRange = capabilities.m_bIsDepthRangeZeroToOne ? ezClipSpaceDepthRange::ZeroToOne : ezClipSpaceDepthRange::MinusOneToOne;
    ezClipSpaceYMode::Enum clipSpaceYMode = capabilities.m_bIsClipSpaceYInverted ? ezClipSpaceYMode::Flipped : ezClipSpaceYMode::Regular;

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
        EZ_REPORT_FAILURE("Invalid Camera Mode {0}", (int)m_eProjectionMode);
    }
  }

  void spCamera::CacheViewMatrix()
  {
    if (!m_bIsDirty)
      return;

    m_CachedViewMatrix = ezGraphicsUtils::CreateViewMatrix(GetPosition(), GetForward(), GetLeft(), GetUp(), ezHandedness::LeftHanded);
  }

  void spCamera::UpdateView()
  {
    if (m_pRenderView == nullptr)
      return;

    CacheProjectionMatrix();
    CacheViewMatrix();

    m_pRenderView->SetRenderGroup(m_eRenderGroupMask);
    m_pRenderView->SetCullingMode(m_bCullingEnabled ? spRenderViewCullingMode::Frustum : spRenderViewCullingMode::None);
    m_pRenderView->SetUsage(m_eRenderViewUsage);

    {
      auto viewData = m_pRenderView->GetDataBuffer().Write();
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
      viewData->m_InverseProjection = m_CachedProjectionMatrix.GetInverse();
      viewData->m_View = m_CachedViewMatrix;
      viewData->m_InverseView = m_CachedViewMatrix.GetInverse();
    }
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Camera_Camera);
