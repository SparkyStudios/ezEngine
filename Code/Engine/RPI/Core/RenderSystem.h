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

  /// \brief A struct used to create and hold references to render nodes.
  /// Theses references are most of the time indices to a render node from
  /// an array.
  struct SP_RHI_DLL spRenderNodeReference
  {
  public:
    EZ_DECLARE_POD_TYPE();

    /// \brief Makes an invalid reference.
    static spRenderNodeReference MakeInvalid();

    /// \brief Makes a reference to a node.
    /// \param [in] iReference The reference to the node.
    explicit spRenderNodeReference(ezInt32 iReference);

    /// \brief Gets the reference. It's typically the index of the render node
    /// in the collection it is associated with.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezInt32 GetRef() const { return m_iRef; }

    /// \brief Checks if the reference is invalid.
    EZ_NODISCARD EZ_ALWAYS_INLINE bool IsInvalid() const { return m_iRef == -1; }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator==(const spRenderNodeReference& rhs) const
    {
      return m_iRef == rhs.m_iRef;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator!=(const spRenderNodeReference& rhs) const
    {
      return m_iRef != rhs.m_iRef;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE bool operator<(const spRenderNodeReference& rhs) const
    {
      return m_iRef < rhs.m_iRef;
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE spRenderNodeReference operator+(const ezInt32 rhs) const
    {
      return spRenderNodeReference(m_iRef + rhs);
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE spRenderNodeReference operator*(const ezInt32 rhs) const
    {
      return spRenderNodeReference(m_iRef * rhs);
    }

  private:
    ezInt32 m_iRef{-1};
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

    void AddRenderObject(spRenderObject* pRenderObject);
    void RemoveRenderObject(spRenderObject* pRenderObject);

    EZ_ALWAYS_INLINE spConcurrentCollector<spRenderView*>& GetRenderViewCollector() { return m_RenderViewCollector; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderView*>& GetRenderViewCollector() const { return m_RenderViewCollector; }

    EZ_ALWAYS_INLINE spConcurrentCollector<spRenderFeature*>& GetRenderFeatureCollector() { return m_RenderFeatureCollector; }
    EZ_NODISCARD EZ_ALWAYS_INLINE const spConcurrentCollector<spRenderFeature*>& GetRenderFeatureCollector() const { return m_RenderFeatureCollector; }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spCompositor> GetCompositor() const { return m_pCompositor; }

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
    spConcurrentCollector<spRenderFeature*> m_RenderFeatureCollector;

    ezSharedPtr<spCompositor> m_pCompositor{nullptr};
  };
} // namespace RPI
