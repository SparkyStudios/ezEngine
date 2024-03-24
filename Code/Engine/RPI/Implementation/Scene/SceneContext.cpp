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

#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderContext.h>
#include <RPI/Pipeline/RenderPipeline.h>
#include <RPI/Scene/SceneContext.h>

using namespace RHI;

namespace RPI
{
  ezEvent<const spSceneContextCollectEvent&, ezMutex> spSceneContext::s_CollectEvent;
  ezEvent<const spSceneContextExtractEvent&, ezMutex> spSceneContext::s_ExtractEvent;
  ezEvent<const spSceneContextPrepareEvent&, ezMutex> spSceneContext::s_PrepareEvent;
  ezEvent<const spSceneContextDrawEvent&, ezMutex> spSceneContext::s_DrawEvent;

  spSceneContext::spSceneContext(spDevice* pDevice)
    : m_pDevice(pDevice)
  {
    m_pCommandList = pDevice->GetResourceFactory()->CreateCommandList(spCommandListDescription());
    m_pFence = pDevice->GetResourceFactory()->CreateFence(spFenceDescription(false));

    m_pRenderContext = EZ_NEW(pDevice->GetAllocator(), spRenderContext, pDevice);
  }

  void spSceneContext::AddPipeline(ezUniquePtr<spRenderPipeline> pPipeline)
  {
    m_RenderPipelines.PushBack(std::move(pPipeline));
  }

  void spSceneContext::Collect()
  {
    m_RenderViewCollector.Clear();
    m_RenderStageCollector.Clear();
    m_RenderFeatureCollector.Clear();

    // Collect render views and extractors from providers
    spSceneContextCollectEvent event{};
    event.m_Type = spSceneContextCollectEvent::Type::Collect;
    event.m_pSceneContext = this;
    s_CollectEvent.Broadcast(event);

    // Close the collectors
    m_RenderViewCollector.Close();
    m_RenderStageCollector.Close();
    m_RenderFeatureCollector.Close();

    // Trigger the "after collect" event
    event.m_Type = spSceneContextCollectEvent::Type::AfterCollect;
    s_CollectEvent.Broadcast(event);
  }

  void spSceneContext::Extract()
  {
    // Prepare views
    for (ezUInt32 i = 0, l = m_RenderViewCollector.GetCount(); i < l; i++)
    {
      // Update view index
      spRenderView* view = m_RenderViewCollector[i];
      view->SetIndex(i);
    }

    // Trigger the "before extract" event
    spSceneContextExtractEvent event{};
    event.m_Type = spSceneContextExtractEvent::Type::BeforeExtract;
    event.m_pSceneContext = this;
    s_ExtractEvent.Broadcast(event);

    // Extract render data from views
    for (const auto& view : m_RenderViewCollector)
      for (const auto& feature : m_RenderFeatureCollector)
        feature->Extract(this, m_pRenderContext.Borrow(), view);

    // Trigger the "after extract" event
    event.m_Type = spSceneContextExtractEvent::Type::AfterExtract;
    s_ExtractEvent.Broadcast(event);
  }

  void spSceneContext::Prepare()
  {
    spSceneContextPrepareEvent event{};
    event.m_Type = spSceneContextPrepareEvent::Type::BeforePrepare;
    event.m_pSceneContext = this;
    s_PrepareEvent.Broadcast(event);

    // TODO

    // Trigger the "after prepare" event
    event.m_Type = spSceneContextPrepareEvent::Type::AfterPrepare;
    s_PrepareEvent.Broadcast(event);
  }

  void spSceneContext::BeginFrame()
  {
    m_pDevice->BeginFrame();

    Extract();

    m_pRenderContext->SetCommandList(m_pCommandList);
    m_pRenderContext->BeginFrame();

    Prepare();
  }

  void spSceneContext::Draw()
  {
    // Trigger the "before draw" event
    spSceneContextDrawEvent event{};
    event.m_Type = spSceneContextDrawEvent::Type::BeforeDraw;
    event.m_pSceneContext = this;
    s_DrawEvent.Broadcast(event);

    for (ezUInt32 i = 0, l = m_RenderPipelines.GetCount(); i < l; i++)
      m_RenderPipelines[i]->Execute(m_pRenderContext.Borrow());

    // Trigger the "after draw" event
    event.m_Type = spSceneContextDrawEvent::Type::AfterDraw;
    s_DrawEvent.Broadcast(event);
  }

  void spSceneContext::Present()
  {
    m_pDevice->Present();
  }

  void spSceneContext::EndFrame()
  {
    m_pRenderContext->EndFrame(m_pFence);

    Flush();

    m_pDevice->EndFrame();

    Reset();
  }

  void spSceneContext::Flush()
  {
  }

  void spSceneContext::Reset()
  {
    m_pRenderContext->Reset();
  }

  void spSceneContext::WaitForIdle()
  {
    m_pDevice->WaitForFence(m_pFence);
    m_pDevice->WaitForIdle();

    m_pDevice->ResetFence(m_pFence);
  }

  void spSceneContext::CleanUp()
  {
    for (auto& pipeline : m_RenderPipelines)
      pipeline->CleanUp();
  }

  void spSceneContext::AddRenderObject(spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "RenderObject cannot be nullptr");

    for (const auto& feature : m_RenderFeatureCollector)
    {
      ezHybridArray<const ezRTTI*, 8> supportedTypes;
      feature->GetSupportedRenderObjectTypes(supportedTypes);

      if (!supportedTypes.Contains(pRenderObject->GetDynamicRTTI()))
        continue;

      if (feature->TryAddRenderObject(pRenderObject))
        break;
    }
  }

  void spSceneContext::RemoveRenderObject(spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "RenderObject cannot be nullptr");

    if (pRenderObject->m_pRenderFeature == nullptr)
      return;

    pRenderObject->m_pRenderFeature->RemoveRenderObject(pRenderObject);
  }
} // namespace RPI
