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

#include <RPI/Camera/CameraSlot.h>

namespace RPI
{
  class spRenderSystem;
  class spRenderContext;
  class spRenderFeature;
  class spRenderer;
  class spCamera;

  class SP_RPI_DLL spCompositorEntryPoint : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spCompositorEntryPoint, ezReflectedClass);

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsConnected() const
    {
      return m_iPinIndex != -1;
    }

    ezResult Serialize(ezStreamWriter& inout_stream) const;
    ezResult Deserialize(ezStreamReader& inout_stream);

  protected:
    ezInt16 m_iPinIndex{-1};
    ezUInt8 m_uiNumConnections{0};
  };

  class SP_RPI_DLL spCompositor : public ezReflectedClass, public ezRefCounted
  {
    EZ_ADD_DYNAMIC_REFLECTION(spCompositor, ezReflectedClass);
    EZ_DISALLOW_COPY_AND_ASSIGN(spCompositor);

  public:
    spCompositor() = default;
    ~spCompositor() override;

#pragma region Camera Slot Management

    spCameraSlotHandle CreateCameraSlot(ezStringView sName);
    void DeleteCameraSlot(const spCameraSlotHandle& hSlot);
    EZ_NODISCARD const spCameraSlot* GetCameraSlot(const spCameraSlotHandle& hSlot) const;

    EZ_NODISCARD spCameraSlotHandle GetCameraSlotByName(ezStringView sName) const;

    void AssignSlotToCamera(const spCameraSlotHandle& hSlot, spCamera* pCamera);

    EZ_NODISCARD spCamera* GetCameraBySlot(const spCameraSlotHandle& hSlot) const;

#pragma endregion

#pragma region Renderer Settings

    EZ_ALWAYS_INLINE void SetEnableHDR(bool bEnable) { m_bEnableHDR = bEnable; }
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsHDR() const { return m_bEnableHDR; }

    EZ_ALWAYS_INLINE void SetViewportSize(const ezRectU32& rect) { m_ViewportSize = rect; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezRectU32 GetViewportSize() const { return m_ViewportSize; }

    EZ_ALWAYS_INLINE void SetRenderSize(const ezRectU32& rect) { m_RenderSize = rect; }
    EZ_NODISCARD EZ_ALWAYS_INLINE ezRectU32 GetRenderSize() const { return m_RenderSize; }

#pragma endregion

    // Cached values
    spRenderer* m_pGameRenderer{nullptr};
    spRenderer* m_pEditorRenderer{nullptr};
    spRenderer* m_pPreviewRenderer{nullptr};

  private:
    using spCameraSlotTable = ezIdTable<spCameraSlotHandle::IdType, spCameraSlot*>;

    spCameraSlotTable m_CameraSlots;
    ezArrayMap<ezStringView, spCameraSlotHandleId> m_CameraSlotsByName;
    ezArrayMap<spCameraSlotHandleId, spCamera*> m_AssignedCameraSlots;

    ezRectU32 m_ViewportSize;
    ezRectU32 m_RenderSize;

    bool m_bEnableHDR{false};
  };
} // namespace RPI
