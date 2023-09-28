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

#include <Foundation/Configuration/CVar.h>

#include <RPI/Core/RenderSystem.h>

ezCVarBool cvar_RenderingMultithreading("Rendering.Multithreading", true, ezCVarFlags::Default, "Enables multi-threaded data update and rendering.");

namespace RPI
{
  EZ_IMPLEMENT_SINGLETON(spRenderSystem);

  ezEvent<const spRenderSystemCollectEvent&, ezMutex> spRenderSystem::s_CollectEvent;
  ezEvent<const spRenderSystemExtractEvent&, ezMutex> spRenderSystem::s_ExtractEvent;

  ezUInt64 spRenderSystem::s_uiFrameCount = 0;

  bool spRenderSystem::IsMultiThreadedRendering()
  {
    return cvar_RenderingMultithreading;
  }

  spRenderSystem::spRenderSystem()
    : m_SingletonRegistrar(this)
  {
  }

  void spRenderSystem::Startup(ezUniquePtr<RHI::spDevice>&& pDevice)
  {
    if (m_bInitialized)
      return;

    m_pDevice = std::move(pDevice);

    m_pRenderThread = EZ_DEFAULT_NEW(spRenderThread);
    m_pRenderThread->Start();

    m_pSceneContext = EZ_NEW(m_pDevice->GetAllocator(), spSceneContext, m_pDevice.Borrow());

    // TODO: Load the compositor from the resource instead
    m_pCompositor = EZ_NEW(m_pDevice->GetAllocator(), spCompositor, this);

    m_bInitialized = true;
  }

  void spRenderSystem::Shutdown()
  {
    if (!m_bInitialized)
      return;

    m_bInitialized = false;

    m_pSceneContext = nullptr;

    m_pRenderThread->Stop();
    m_pRenderThread = nullptr;

    m_pDevice->Destroy();
    m_pDevice = nullptr;
  }

  void spRenderSystem::Collect(const spRenderContext* pRenderContext)
  {
    if (pRenderContext == nullptr)
      return;

    m_RenderViewCollector.Clear();
    m_RenderStageCollector.Clear();
    m_RenderFeatureExtractorCollector.Clear();

    // Collect render views and extractors from providers
    spRenderSystemCollectEvent event{};
    event.m_Type = spRenderSystemCollectEvent::Type::Collect;
    event.m_pRenderContext = pRenderContext;
    s_CollectEvent.Broadcast(event);

    // Close the collectors
    m_RenderViewCollector.Close();
    m_RenderStageCollector.Close();
    m_RenderFeatureExtractorCollector.Close();

    // Trigger the "after collect" event
    event.m_Type = spRenderSystemCollectEvent::Type::AfterCollect;
    s_CollectEvent.Broadcast(event);
  }

  void spRenderSystem::Extract(const spRenderContext* pRenderContext)
  {
    if (pRenderContext == nullptr)
      return;

    // Prepare views
    for (ezInt32 index = 0; index < m_RenderViewCollector.GetCount(); index++)
    {
      // Update view index
      spRenderView* view = m_RenderViewCollector[index];
      view->SetIndex(index);
    }

    // Trigger the "before extract" event
    spRenderSystemExtractEvent event{};
    event.m_Type = spRenderSystemExtractEvent::Type::BeforeExtract;
    event.m_pRenderContext = pRenderContext;
    s_ExtractEvent.Broadcast(event);

    // Extract render data from views
    for (const auto& view : m_RenderViewCollector)
      for (const auto& extractor : m_RenderFeatureExtractorCollector)
        extractor->Extract(pRenderContext, view);

    // Trigger the "after extract" event
    event.m_Type = spRenderSystemExtractEvent::Type::AfterExtract;
    s_ExtractEvent.Broadcast(event);
  }
} // namespace RPI
