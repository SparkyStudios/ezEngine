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

#include <RPI/Renderers/SceneRenderer.h>
#include <RPI/Scene/SceneContext.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSceneRenderer, 1, ezRTTIDefaultAllocator<spSceneRenderer>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spSceneRenderer::Render()
  {
    const auto cl = GetSceneContext()->GetRenderContext()->GetCommandList();
    const auto pFramebuffer = GetSceneContext()->GetDevice()->GetMainSwapchain()->GetFramebuffer();

    cl->PushDebugGroup("Scene Renderer");
    {
      auto clPushRestore = cl->PushRestoreFramebuffer(m_pRenderTarget->GetFramebuffer());

      SUPER::Render();

      cl->CopyTexture(
        m_pRenderTarget->GetTexture(),
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(pFramebuffer->GetColorTargets()[0]));
    }
    cl->PopDebugGroup();
  }

  void spSceneRenderer::Initialize(const spSceneContext* pSceneContext)
  {
    const auto pCompositor = spRenderSystem::GetSingleton()->GetCompositor();
    const ezRectU32 renderSize = pCompositor->GetRenderSize();

    m_pRenderTarget.Clear();

    RHI::spRenderTargetDescription desc;
    desc.m_eQuality = pCompositor->IsHDR() ? RHI::spRenderTargetQuality::HDR : RHI::spRenderTargetQuality::LDR;
    desc.m_uiWidth = renderSize.width;
    desc.m_uiHeight = renderSize.height;
    desc.m_eSampleCount = RHI::spTextureSampleCount::None; // TODO: Get this from the compositor.
    desc.m_bGenerateMipMaps = false;

    m_pRenderTarget = pSceneContext->GetDevice()->GetResourceFactory()->CreateRenderTarget(desc);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    m_pRenderTarget->SetDebugName("Camera Renderer Render Target");
#endif

    SUPER::Initialize(pSceneContext);
  }

  spSceneRenderer::spSceneRenderer()
    : spParentRenderer("SceneRenderer")
  {
  }
} // namespace RPI
