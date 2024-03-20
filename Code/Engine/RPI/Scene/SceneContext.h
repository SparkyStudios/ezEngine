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

#include <RHI/CommandList.h>
#include <RHI/Device.h>
#include <RHI/Fence.h>

#include <RPI/Core/RenderContext.h>
#include <RPI/Core/RenderStage.h>
#include <RPI/Core/RenderView.h>
#include <RPI/Core/Threading/ConcurrentCollector.h>
#include <RPI/Pipeline/RenderPipeline.h>

#include <RPI/Features/RenderFeature.h>

namespace RPI
{
  class spSceneContext;

  struct spSceneContextCollectEvent
  {
    enum class Type
    {
      Collect,
      AfterCollect
    };

    Type m_Type;
    spSceneContext* m_pSceneContext;
  };

  struct spSceneContextExtractEvent
  {
    enum class Type
    {
      BeforeExtract,
      AfterExtract
    };

    Type m_Type;
    spSceneContext* m_pSceneContext;
  };

  struct spSceneContextPrepareEvent
  {
    enum class Type
    {
      BeforePrepare,
      AfterPrepare
    };

    Type m_Type;
    spSceneContext* m_pSceneContext;
  };

  struct spSceneContextDrawEvent
  {
    enum class Type
    {
      BeforeDraw,
      AfterDraw
    };

    Type m_Type;
    spSceneContext* m_pSceneContext;
  };

  class SP_RPI_DLL spSceneContext
  {
    friend class spVisibilityGroup;

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE static const ezEvent<const spSceneContextCollectEvent&, ezMutex>& GetCollectEvent() { return s_CollectEvent; }

    explicit spSceneContext(RHI::spDevice* pDevice);

    /// \brief Gets the \a spRenderContext which stores drawing data for this scene.
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezUniquePtr<spRenderContext>& GetRenderContext() const { return m_pRenderContext; }

    /// \brief Adds a \a spRenderingPipeline to execute when drawing this \a spSceneContext.
    void AddPipeline(ezUniquePtr<spRenderPipeline> pPipeline);

    void Collect();

    void Extract();

    void Prepare();

    /// \brief Begins a draw operation.
    void BeginFrame();

    /// \brief Draws the current \a spSceneContext on its \a spRenderTarget.
    void Draw();

    void Present();

    /// \brief Ends a draw operation.
    void EndFrame();

    void Flush();

    void Reset();

    /// \brief Blocks the calling thread until this \a spSceneContext
    /// is idle (all outstanding draw operations are completed).
    void WaitForIdle();

    void CleanUp();

    EZ_ALWAYS_INLINE spConcurrentCollector<spRenderView*>& GetRenderViewCollector() { return m_RenderViewCollector; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderView*>& GetRenderViewCollector() const { return m_RenderViewCollector; }

    EZ_ALWAYS_INLINE spConcurrentCollector<spRenderFeature*>& GetRenderFeatureCollector() { return m_RenderFeatureCollector; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderFeature*>& GetRenderFeatureCollector() const { return m_RenderFeatureCollector; }

  private:
    static ezEvent<const spSceneContextCollectEvent&, ezMutex> s_CollectEvent;
    static ezEvent<const spSceneContextExtractEvent&, ezMutex> s_ExtractEvent;
    static ezEvent<const spSceneContextPrepareEvent&, ezMutex> s_PrepareEvent;
    static ezEvent<const spSceneContextDrawEvent&, ezMutex> s_DrawEvent;

    void AddRenderObject(spRenderObject* pRenderObject);
    void RemoveRenderObject(spRenderObject* pRenderObject);

    RHI::spDevice* m_pDevice{nullptr};

    ezSharedPtr<RHI::spCommandList> m_pCommandList{nullptr};
    ezSharedPtr<RHI::spFence> m_pFence{nullptr};

    ezUniquePtr<spRenderContext> m_pRenderContext{nullptr};

    spConcurrentCollector<spRenderView*> m_RenderViewCollector;
    spConcurrentCollector<spRenderStage*> m_RenderStageCollector;
    spConcurrentCollector<spRenderFeature*> m_RenderFeatureCollector;

    ezDynamicArray<ezUniquePtr<spRenderPipeline>> m_RenderPipelines;
  };
} // namespace RPI
