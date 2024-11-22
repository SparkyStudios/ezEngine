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

#include <RPI/Core/RenderObject.h>
#include <RPI/Core/RenderSystem.h>
#include <RPI/Core/RenderView.h>
#include <RPI/Meshes/MeshRenderObject.h>
#include <RPI/Stages/OpaqueRenderStage.h>

#include <RHI/Device.h>
#include <RHI/Texture.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spOpaqueRenderStage, 1, ezRTTIDefaultAllocator<spOpaqueRenderStage>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  bool spOpaqueRenderStage::IsActiveForRenderObject(const spRenderObject* pRenderObject) const
  {
    // TODO: Get the material from the render object and check if it is active for this render stage (not transparent).
    return pRenderObject->IsInstanceOf<spMeshRenderObject>();
  }

  RHI::spRenderingState spOpaqueRenderStage::GetRenderingState(const spRenderObject* pRenderObject) const
  {
    RHI::spRenderingState renderingState{};

    renderingState.m_BlendState.m_BlendColor = ezColorGammaUB(0x00, 0x00, 0x00);
    renderingState.m_BlendState.m_AttachmentStates.EnsureCount(GBuffer::kNumTargets);
    renderingState.m_BlendState.m_AttachmentStates[0] = RHI::spBlendAttachment::Disabled;
    renderingState.m_BlendState.m_AttachmentStates[1] = RHI::spBlendAttachment::Disabled;
    renderingState.m_BlendState.m_AttachmentStates[2] = RHI::spBlendAttachment::Disabled;
    renderingState.m_BlendState.m_AttachmentStates[3] = RHI::spBlendAttachment::Disabled;

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
    auto& gBuffer = m_ColorTextures[pRenderView];

    const auto* pDevice = spRenderSystem::GetSingleton()->GetDevice();

    const auto& viewport = pRenderView->GetViewport();

    if (pDepthTarget == nullptr)
    {
      const auto& desc = RHI::spTextureDescription::Texture2D(viewport.width, viewport.height, 1, 1, RHI::spPixelFormat::D32FloatS8UInt, RHI::spTextureUsage::DepthStencil);
      pDepthTarget = pDevice->GetResourceFactory()->CreateTexture(desc);
    }

    if (gBuffer.GetDiffuseEmissive() == nullptr)
    {
      const auto& desc = RHI::spTextureDescription::Texture2D(viewport.width, viewport.height, 1, 1, RHI::spPixelFormat::R8G8B8A8UNorm, RHI::spTextureUsage::RenderTarget | RHI::spTextureUsage::Sampled | RHI::spTextureUsage::Storage);
      gBuffer.GetDiffuseEmissive() = pDevice->GetResourceFactory()->CreateTexture(desc);
    }

    if (gBuffer.GetNormalMaterialIndex() == nullptr)
    {
      const auto& desc = RHI::spTextureDescription::Texture2D(viewport.width, viewport.height, 1, 1, RHI::spPixelFormat::R16G16B16A16Float, RHI::spTextureUsage::RenderTarget | RHI::spTextureUsage::Sampled | RHI::spTextureUsage::Storage);
      gBuffer.GetNormalMaterialIndex() = pDevice->GetResourceFactory()->CreateTexture(desc);
    }

    if (gBuffer.GetMetalnessRoughnessOcclusionCavity() == nullptr)
    {
      const auto& desc = RHI::spTextureDescription::Texture2D(viewport.width, viewport.height, 1, 1, RHI::spPixelFormat::R8G8B8A8UNorm, RHI::spTextureUsage::RenderTarget | RHI::spTextureUsage::Sampled | RHI::spTextureUsage::Storage);
      gBuffer.GetMetalnessRoughnessOcclusionCavity() = pDevice->GetResourceFactory()->CreateTexture(desc);
    }

    if (gBuffer.GetVelocity() == nullptr)
    {
      const auto& desc = RHI::spTextureDescription::Texture2D(viewport.width, viewport.height, 1, 1, RHI::spPixelFormat::R32G32Float, RHI::spTextureUsage::RenderTarget | RHI::spTextureUsage::Sampled | RHI::spTextureUsage::Storage);
      gBuffer.GetVelocity() = pDevice->GetResourceFactory()->CreateTexture(desc);
    }

    RHI::spResourceHandle targets[] = {
      gBuffer.GetDiffuseEmissive()->GetHandle(),
      gBuffer.GetNormalMaterialIndex()->GetHandle(),
      gBuffer.GetMetalnessRoughnessOcclusionCavity()->GetHandle(),
      gBuffer.GetVelocity()->GetHandle()};

    const RHI::spFramebufferDescription desc(pDepthTarget->GetHandle(), ezMakeArrayPtr(targets));
    m_RenderViewFramebuffers[pRenderView] = pDevice->GetResourceFactory()->CreateFramebuffer(desc);
  }

  spOpaqueRenderStage::spOpaqueRenderStage()
    : spRenderStage("Opaque")
  {
    // TODO: Add a filter after having the material system
    m_pSortMode = EZ_DEFAULT_NEW(spFrontToBackSortMode);
  }
} // namespace RPI
