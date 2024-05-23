// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <RPI/Core/RenderSystem.h>
#include <RPI/Core/RenderView.h>
#include <RPI/Stages/OpaqueRenderStage.h>

#include <RHI/Device.h>
#include <RHI/Texture.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spOpaqueRenderStage, 1, ezRTTIDefaultAllocator<spOpaqueRenderStage>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spOpaqueRenderStage::Filter(const spRenderView& renderView, const ezArrayPtr<spRenderObject>& renderObjects, ezArrayPtr<spRenderObject>& out_filteredRenderObjects, ezArrayPtr<spRenderObject>& out_unfilteredRenderObjects)
  {
  }

  void spOpaqueRenderStage::Sort(const spRenderView& renderView, const ezArrayPtr<spRenderObject>& renderObjects, ezArrayPtr<spRenderObject>& out_sortedRenderObjects)
  {
  }

  bool spOpaqueRenderStage::IsActiveForRenderObject(const spRenderObject* pRenderObject) const
  {
    // TODO: Get the material from the render object and check if it is active for this render stage (not transparent).
    return true;
  }

  RHI::spRenderingState spOpaqueRenderStage::GetRenderingState(const spRenderObject* pRenderObject) const
  {
    RHI::spRenderingState renderingState{};

    renderingState.m_BlendState = RHI::spBlendState::SingleDisabled;

    renderingState.m_DepthState = RHI::spDepthState::Less;

    renderingState.m_RasterizerState = RHI::spRasterizerState::Default;

    renderingState.m_StencilState.m_bEnabled = true;
    renderingState.m_StencilState.m_uiWriteMask = RHI::spColorWriteMask::All;
    renderingState.m_StencilState.m_uiReadMask = RHI::spColorWriteMask::All;
    renderingState.m_StencilState.m_uiReference = 1;
    renderingState.m_StencilState.m_Back.m_eComparison = RHI::spDepthStencilComparison::Always;
    renderingState.m_StencilState.m_Back.m_eDepthFail = RHI::spStencilOperation::Keep;
    renderingState.m_StencilState.m_Back.m_eFail = RHI::spStencilOperation::Keep;
    renderingState.m_StencilState.m_Back.m_ePass = RHI::spStencilOperation::Replace;
    renderingState.m_StencilState.m_Front.m_eComparison = RHI::spDepthStencilComparison::Always;
    renderingState.m_StencilState.m_Front.m_eDepthFail = RHI::spStencilOperation::Keep;
    renderingState.m_StencilState.m_Front.m_eFail = RHI::spStencilOperation::Keep;
    renderingState.m_StencilState.m_Front.m_ePass = RHI::spStencilOperation::Replace;

    return renderingState;
  }

  void spOpaqueRenderStage::CreateOutputFramebuffer(const spRenderView* pRenderView)
  {
    if (m_RenderViewFramebuffers.Contains(pRenderView))
      return;

    auto& pDepthTarget = m_DepthStencilTextures[pRenderView];
    auto& pColorTarget = m_ColorTextures[pRenderView];

    const auto* pDevice = spRenderSystem::GetSingleton()->GetDevice();

    if (pDepthTarget == nullptr)
    {
      const auto& viewport = pRenderView->GetViewport();

      const auto& depthDesc = RHI::spTextureDescription::Texture2D(viewport.width * 2, viewport.height * 2, 1, 1, RHI::spPixelFormat::D32FloatS8UInt, RHI::spTextureUsage::DepthStencil);
      pDepthTarget = pDevice->GetResourceFactory()->CreateTexture(depthDesc);
    }

    if (pColorTarget == nullptr)
    {
      const auto& viewport = pRenderView->GetViewport();

      const auto& colorDesc = RHI::spTextureDescription::Texture2D(viewport.width * 2, viewport.height * 2, 1, 1, RHI::spPixelFormat::R8G8B8A8UNorm, RHI::spTextureUsage::RenderTarget | RHI::spTextureUsage::Sampled | RHI::spTextureUsage::Storage);
      pColorTarget = pDevice->GetResourceFactory()->CreateTexture(colorDesc);
    }

    const RHI::spFramebufferDescription desc(pDepthTarget->GetHandle(), pColorTarget->GetHandle());
    m_RenderViewFramebuffers[pRenderView] = pDevice->GetResourceFactory()->CreateFramebuffer(desc);
  }

  spOpaqueRenderStage::spOpaqueRenderStage()
    : spRenderStage("Opaque")
  {
  }
} // namespace RPI
