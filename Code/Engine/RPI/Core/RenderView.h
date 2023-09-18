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

#include <Foundation/Math/Frustum.h>

namespace RPI
{
  /// \brief Render View Data.
  /// This struct contains all the data needed to render a single view. Data is
  /// shared with shaders.
  struct alignas(16) spRenderViewData
  {
    ezVec3 m_Position;
    float m_NearClipPlane;

    ezVec3 m_Rotation;
    float m_FarClipPlane;

    ezVec3 m_Direction;
    float m_FieldOfView;

    ezVec2 m_ViewportSize;
    ezVec2 m_InverseViewportSize;

    ezVec2 m_RenderSize;
    ezVec2 m_InverseRenderSize;

    ezColor m_ClearColor;

    ezMat4 m_Projection;
    ezMat4 m_InverseProjection;

    ezMat4 m_View;
    ezMat4 m_InverseView;
  };

  /// \brief Specifies how the render view should cull render objects.
  struct SP_RPI_DLL spRenderViewCullingMode
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

  class SP_RPI_DLL spRenderView
  {
  public:
    spRenderView() = default;
    ~spRenderView() = default;

    EZ_NODISCARD EZ_ALWAYS_INLINE const spRenderViewData& GetData() const { return m_Data; }

    EZ_ALWAYS_INLINE void SetData(const spRenderViewData& data) { m_Data = data; }

    /// \brief Gets the index of this view in the list of collected views in the \a spRenderSystem.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezInt32 GetIndex() const { return m_iIndex; }

    /// \brief Sets the index this view has in the list of collected views in the \a spRenderSystem.
    EZ_ALWAYS_INLINE void SetIndex(ezInt32 index) { m_iIndex = index; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezBitflags<spRenderGroupMask>& GetRenderGroup() const { return m_eRenderGroup; }

    EZ_ALWAYS_INLINE void SetRenderGroup(const ezBitflags<spRenderGroupMask>& eGroupMask) { m_eRenderGroup = eGroupMask; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezEnum<spRenderViewCullingMode>& GetCullingMode() const { return m_eCullingMode; }

    EZ_ALWAYS_INLINE void SetCullingMode(const ezEnum<spRenderViewCullingMode>& eCullingMode) { m_eCullingMode = eCullingMode; }

  private:
    friend class spRenderSystem;
    friend class spRenderContext;

    spRenderViewData m_Data{};

    ezInt32 m_iIndex{-1};
    ezBitflags<spRenderGroupMask> m_eRenderGroup{spRenderGroupMask::Default};
    ezEnum<spRenderViewCullingMode> m_eCullingMode{spRenderViewCullingMode::Default};
  };
} // namespace RPI
