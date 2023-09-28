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

#include <RPI/Composition/Compositor.h>
#include <RPI/Core/RenderStage.h>
#include <RPI/Core/RenderView.h>
#include <RPI/Core/Threading/ConcurrentCollector.h>
#include <RPI/Core/Threading/RenderThread.h>
#include <RPI/Features/RenderFeatureExtractor.h>
#include <RPI/Scene/SceneContext.h>

namespace RPI
{
  struct spRenderSystemCollectEvent
  {
    enum class Type
    {
      Collect,
      AfterCollect
    };

    Type m_Type;
    const spRenderContext* m_pRenderContext;
  };

  struct spRenderSystemExtractEvent
  {
    enum class Type
    {
      BeforeExtract,
      AfterExtract
    };

    Type m_Type;
    const spRenderContext* m_pRenderContext;
  };

  struct spRenderSystemPrepareEvent
  {
    enum class Type
    {
      BeforePrepare,
      AfterPrepare
    };

    Type m_Type;
  };

  struct spRenderSystemDrawEvent
  {
    enum class Type
    {
      BeforeDraw,
      AfterDraw
    };

    Type m_Type;
  };

  class SP_RPI_DLL spRenderSystem
  {
    EZ_DECLARE_SINGLETON(spRenderSystem);

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE static const ezEvent<const spRenderSystemCollectEvent&, ezMutex>& GetCollectEvent() { return s_CollectEvent; }

    EZ_NODISCARD static bool IsMultiThreadedRendering();

    EZ_NODISCARD EZ_ALWAYS_INLINE static ezUInt64 GetFrameCount() { return s_uiFrameCount; }

    EZ_NODISCARD EZ_FORCE_INLINE static ezUInt32 GetDataIndexForExtraction() { return IsMultiThreadedRendering() ? (s_uiFrameCount & 1) : 0; }

    EZ_NODISCARD EZ_FORCE_INLINE static ezUInt32 GetDataIndexForRendering() { return IsMultiThreadedRendering() ? ((s_uiFrameCount + 1) & 1) : 0; }

    spRenderSystem();

    void Startup(ezUniquePtr<RHI::spDevice>&& pDevice);
    void Shutdown();

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezUniquePtr<RHI::spDevice>& GetDevice() const { return m_pDevice; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezUniquePtr<spSceneContext>& GetSceneContext() const { return m_pSceneContext; }

    EZ_NODISCARD EZ_ALWAYS_INLINE const ezUniquePtr<spRenderThread>& GetRenderThread() const { return m_pRenderThread; }

    void Collect(const spRenderContext* pRenderContext);

    void Extract(const spRenderContext* pRenderContext);

    void Prepare(spRenderContext* pRenderContext);

    void Draw(spRenderContext* pRenderContext);

    void Flush(spRenderContext* pRenderContext);

    void Reset();

    EZ_ALWAYS_INLINE spConcurrentCollector<spRenderView*>& GetRenderViewCollector() { return m_RenderViewCollector; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderView*>& GetRenderViewCollector() const { return m_RenderViewCollector; }

    EZ_ALWAYS_INLINE spConcurrentCollector<spRenderFeatureExtractor*>& GetRenderFeatureExtractorCollector() { return m_RenderFeatureExtractorCollector; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderFeatureExtractor*>& GetRenderFeatureExtractorCollector() const { return m_RenderFeatureExtractorCollector; }

  private:
    static ezEvent<const spRenderSystemCollectEvent&, ezMutex> s_CollectEvent;
    static ezEvent<const spRenderSystemExtractEvent&, ezMutex> s_ExtractEvent;

    static ezUInt64 s_uiFrameCount;

    bool m_bInitialized{false};

    ezUniquePtr<RHI::spDevice> m_pDevice{nullptr};

    ezUniquePtr<spRenderThread> m_pRenderThread{nullptr};
    ezUniquePtr<spSceneContext> m_pSceneContext{nullptr};

    spConcurrentCollector<spRenderView*> m_RenderViewCollector;
    spConcurrentCollector<spRenderStage*> m_RenderStageCollector;
    spConcurrentCollector<spRenderFeatureExtractor*> m_RenderFeatureExtractorCollector;

    ezSharedPtr<spCompositor> m_pCompositor{nullptr};
  };
} // namespace RPI
