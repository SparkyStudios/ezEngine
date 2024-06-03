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
#include <RPI/Core/RenderSystem.h>
#include <RPI/Scene/SceneContext.h>

#include <Core/World/World.h>

using namespace RHI;

namespace RPI
{
  spRenderContext::spRenderContext(const spSceneContext* pSceneContext)
    : m_pSceneContext(pSceneContext)
    , m_Data()
  {
    m_pShaderManager = EZ_DEFAULT_NEW(spShaderManager);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    m_FrameDataBuffer.SetDebugName("Buffer_PerFrame");
#endif
  }

  spRenderContext::~spRenderContext()
  {
    m_pCommandList.Clear();
    m_pShaderManager.Clear();
  }

  void spRenderContext::SetCommandList(ezSharedPtr<spCommandList> pCommandList)
  {
    if (m_pCommandList == pCommandList)
      return;

    m_pCommandList = pCommandList;
    Reset();
  }

  void spRenderContext::SetCommandList(const spCommandListDescription& description)
  {
    m_pCommandList = m_pSceneContext->GetDevice()->GetResourceFactory()->CreateCommandList(description);
    Reset();
  }

  void spRenderContext::BeginFrame()
  {
    {
      const auto pFrameData = m_FrameDataBuffer.Write();
      const auto pRenderSystem = spRenderSystem::GetSingleton();
      const auto pCompositor = pRenderSystem->GetCompositor();

      pFrameData->m_fGlobalTime = ezClock::GetGlobalClock()->GetAccumulatedTime().AsFloatInSeconds();
      pFrameData->m_fCurrentTime = m_pSceneContext->GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
      pFrameData->m_fDeltaTime = m_pSceneContext->GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds();
      pFrameData->m_uiFrame = spRenderSystem::GetFrameCount();

      pFrameData->m_vRenderSize = pCompositor->GetRenderSize().GetExtents().Cast<float>();
      pFrameData->m_vInverseRenderSize = {1.0f / pFrameData->m_vRenderSize.x, 1.0f / pFrameData->m_vRenderSize.y};
      pFrameData->m_vViewportSize = pCompositor->GetViewportSize().GetExtents().Cast<float>();
      pFrameData->m_vInverseViewportSize = {1.0f / pFrameData->m_vViewportSize.x, 1.0f / pFrameData->m_vViewportSize.y};

      // TODO
      pFrameData->m_uiOptions = 0;
      pFrameData->m_bHDREnabled = pCompositor->IsHDR();
      pFrameData->m_fHDRMaxNits = 0;
      pFrameData->m_fHDRWhitePoint = 0;
    }

    m_pCommandList->Begin();
  }

  void spRenderContext::EndFrame(ezSharedPtr<spFence> pFence)
  {
    m_pCommandList->End();

    m_pSceneContext->GetDevice()->SubmitCommandList(m_pCommandList, pFence);

    m_FrameDataBuffer.Swap();
  }

  void spRenderContext::Reset()
  {
    m_pCommandList->Reset();
  }

  const spRenderContextData& spRenderContext::GetExtractionData() const
  {
    return m_Data[spRenderSystem::GetDataIndexForExtraction()];
  }

  spRenderContextData& spRenderContext::GetExtractionData()
  {
    return m_Data[spRenderSystem::GetDataIndexForExtraction()];
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_RenderContext);
