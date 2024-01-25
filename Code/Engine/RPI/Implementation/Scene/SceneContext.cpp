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
  spSceneContext::spSceneContext(spDevice* pDevice)
    : m_pDevice(pDevice)
  {
    m_pCommandList = pDevice->GetResourceFactory()->CreateCommandList(spCommandListDescription());
    m_pFence = pDevice->GetResourceFactory()->CreateFence(spFenceDescription(false));

    m_pRenderContext = EZ_NEW(pDevice->GetAllocator(), spRenderContext, pDevice);
    m_pRenderContext->SetCommandList(m_pCommandList);
  }

  void spSceneContext::AddPipeline(spRenderPipeline* pPipeline)
  {
    m_RenderPipelines.PushBack(pPipeline);
  }

  void spSceneContext::BeginFrame()
  {
    m_pDevice->BeginFrame();

    m_pRenderContext->BeginFrame();
  }

  void spSceneContext::Draw()
  {
    for (auto it = m_RenderPipelines.GetIterator(); it.IsValid(); it.Next())
      (*it)->Execute(m_pRenderContext.Borrow());
  }

  void spSceneContext::EndFrame()
  {
    m_pRenderContext->EndFrame(m_pFence);
    m_pRenderContext->Reset();

    m_pDevice->EndFrame();
  }

  void spSceneContext::WaitForIdle()
  {
    m_pDevice->WaitForFence(m_pFence);
    m_pDevice->WaitForIdle();

    m_pDevice->ResetFence(m_pFence);
  }
} // namespace RPI
