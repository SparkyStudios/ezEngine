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

#include <RHI/Device.h>

#include <RPI/Camera/CameraSlot.h>
#include <RPI/Composition/Compositor.h>
#include <RPI/Core/Threading/RenderThread.h>

class ezWorld;

namespace RPI
{
  class spCompositor;
  class spSceneContext;
  class spRenderStage;

  class SP_RPI_DLL spRenderSystem
  {
    friend class spSceneContext;

    EZ_DECLARE_SINGLETON(spRenderSystem);

  public:
    [[nodiscard]] static bool IsMultiThreadedRendering();

    [[nodiscard]] EZ_ALWAYS_INLINE static ezUInt64 GetFrameCount() { return s_uiFrameCount; }

    [[nodiscard]] EZ_FORCE_INLINE static ezUInt32 GetDataIndexForExtraction() { return IsMultiThreadedRendering() ? (s_uiFrameCount & 1) : 0; }

    [[nodiscard]] EZ_FORCE_INLINE static ezUInt32 GetDataIndexForRendering() { return IsMultiThreadedRendering() ? ((s_uiFrameCount + 1) & 1) : 0; }

    spRenderSystem();

    void Startup(ezUniquePtr<RHI::spDevice>&& pDevice);
    void Shutdown();

    [[nodiscard]] EZ_ALWAYS_INLINE RHI::spDevice* GetDevice() const { return m_pDevice.Borrow(); }

    [[nodiscard]] EZ_ALWAYS_INLINE spRenderThread* GetRenderThread() const { return m_pRenderThread.Borrow(); }

    [[nodiscard]] ezSharedPtr<spCompositor> GetCompositor() const;

    [[nodiscard]] spSceneContext* GetSceneContextFromWorld(const ezWorld* pWorld) const;

    spSceneContext* CreateSceneForWorld(ezWorld* pWorld);

    void RegisterSceneForWorld(const ezWorld* pWorld, spSceneContext* pSceneContext);

    void UnregisterSceneForWorld(const ezWorld* pWorld);

#pragma region Render Stages

    /// \brief Registers a new render stage in the \a spRenderSystem.
    /// \param pRenderStage The render stage to register.
    void RegisterRenderStage(spRenderStage* pRenderStage);

    /// \brief Unregisters a render stage from the \a spRenderSystem.
    /// \param pRenderStage The render stage to unregister.
    void UnregisterRenderStage(spRenderStage* pRenderStage);

    /// \brief Gets a registered render stage.
    /// \param uiIndex The index of the render stage to get.
    /// \return The render stage at the given index.
    spRenderStage* GetRenderStage(ezUInt32 uiIndex) const;

    /// \brief Gets the number of registered render stages.
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetRenderStageCount() const { return m_RenderStages.GetCount(); }

    /// \brief Gets all registered render stages.
    [[nodiscard]] EZ_ALWAYS_INLINE ezArrayPtr<spRenderStage* const> GetRenderStages() const { return m_RenderStages.GetArrayPtr(); }

#pragma endregion

  private:
    static ezUInt64 s_uiFrameCount;

    bool m_bInitialized{false};

    ezUniquePtr<RHI::spDevice> m_pDevice{nullptr};
    ezUniquePtr<spRenderThread> m_pRenderThread{nullptr};
    ezSharedPtr<spCompositor> m_pCompositor{nullptr};

    ezArrayMap<const ezWorld*, spSceneContext*> m_RegisteredWorldScenes;
    ezHybridArray<spRenderStage*, 16> m_RenderStages;
  };
} // namespace RPI
