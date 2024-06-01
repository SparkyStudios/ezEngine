// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#include <RPI/Core/RenderGroup.h>
#include <RPI/Core/Threading/ConcurrentCollector.h>
#include <RPI/Shaders/ConstantBuffer.h>
#include <RPI/Shaders/ShaderTypes.h>

#include <Foundation/Math/Frustum.h>

namespace RPI
{
  class spRenderObject;

  /// \brief Render View Data.
  ///
  /// This struct contains all the data needed to render a single view. Data is
  /// shared with shaders.
  struct alignas(16) spRenderViewData
  {
    ezVec3 m_Position;
    float m_NearClipPlane;

    ezVec3 m_PreviousPosition;
    float m_FarClipPlane;

    ezVec3 m_Direction;
    float m_ShutterSpeed;

    float m_Exposure;
    float m_Aperture;
    float m_ISO;
    float m_AspectRatio;

    spShaderMat4 m_Projection;
    spShaderMat4 m_InverseProjection;

    spShaderMat4 m_View;
    spShaderMat4 m_InverseView;

    spShaderMat4 m_ViewProjection;
    spShaderMat4 m_InverseViewProjection;
  };

  /// \brief Specifies how the render view should cull render objects.
  struct spRenderViewCullingMode
  {
    typedef ezUInt8 StorageType;

    enum Enum : StorageType
    {
      /// \brief No culling is applied to render objects.
      None = 0,

      /// \brief Culls render objects outside of the render view's frustum.
      Frustum = 1,

      Default = Frustum
    };
  };

  /// \brief Specifies for which purpose the render view should be used.
  struct SP_RPI_DLL spRenderViewUsage
  {
    typedef ezUInt8 StorageType;

    enum Enum : StorageType
    {
      /// \brief The render view will be used for the main rendering.
      Main = EZ_BIT(0),

      /// \brief The render view will be used for shadow map rendering.
      ShadowMapping = EZ_BIT(1),

      /// \brief The render view will be used for occlusion culling.
      Culling = EZ_BIT(2),

      /// \brief The render view will be used for all types of rendering.
      All = Main | ShadowMapping | Culling,

      Default = Main
    };

    struct Bits
    {
      StorageType Main : 1;
      StorageType ShadowMapping : 1;
      StorageType Culling : 1;
    };
  };

  class SP_RPI_DLL spRenderView final : public ezReflectedClass
  {
    friend class spRenderSystem;
    friend class spRenderContext;
    friend class spVisibilityGroup;

    EZ_ADD_DYNAMIC_REFLECTION(spRenderView, ezReflectedClass);

  public:
    spRenderView() = default;
    ~spRenderView() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE const spConstantBuffer<spRenderViewData>& GetDataBuffer() const { return m_RenderViewDataBuffer; }

    void SetData(const spRenderViewData& data);

    /// \brief Gets the index of this view in the list of collected views in the \a spSceneContext.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezInt32 GetIndex() const { return m_iIndex; }

    /// \brief Sets the index this view has in the list of collected views in the \a spSceneContext.
    EZ_ALWAYS_INLINE void SetIndex(ezInt32 index) { m_iIndex = index; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezBitflags<spRenderGroup>& GetRenderGroup() const { return m_eRenderGroup; }

    EZ_ALWAYS_INLINE void SetRenderGroup(const ezBitflags<spRenderGroup>& eGroupMask) { m_eRenderGroup = eGroupMask; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezEnum<spRenderViewCullingMode>& GetCullingMode() const { return m_eCullingMode; }

    EZ_ALWAYS_INLINE void SetCullingMode(const ezEnum<spRenderViewCullingMode>& eCullingMode) { m_eCullingMode = eCullingMode; }

    EZ_ALWAYS_INLINE void SetUsage(const ezBitflags<spRenderViewUsage>& eUsage) { m_eUsage = eUsage; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezBitflags<spRenderViewUsage>& GetUsage() const { return m_eUsage; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderObject*>& GetVisibleRenderObjects() const { return m_VisibleRenderObjects; }

    EZ_NODISCARD ezFrustum GetFrustum() const;

    EZ_ALWAYS_INLINE void SetViewport(const ezRectU32& viewport) { m_Viewport = viewport; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezRectU32& GetViewport() const { return m_Viewport; }

  private:
    ezUInt64 m_uiLastCollectedFrame;

    spConstantBuffer<spRenderViewData> m_RenderViewDataBuffer;

    ezInt32 m_iIndex{-1};
    ezBitflags<spRenderGroup> m_eRenderGroup{spRenderGroup::Default};
    ezEnum<spRenderViewCullingMode> m_eCullingMode{spRenderViewCullingMode::Default};
    ezBitflags<spRenderViewUsage> m_eUsage{spRenderViewUsage::Default};

    spConcurrentCollector<spRenderObject*> m_VisibleRenderObjects;

    ezBoundingBox m_BoundingBox{ezBoundingBox::MakeZero()};
    ezRectU32 m_Viewport{ezRectU32::MakeInvalid()};
  };
} // namespace RPI

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spRenderViewUsage);
