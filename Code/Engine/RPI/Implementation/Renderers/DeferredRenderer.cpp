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

#include <RPI/Renderers/DeferredRenderer.h>
#include <RPI/Stages/OpaqueRenderStage.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spDeferredRenderer, 1, ezRTTIDefaultAllocator<spDeferredRenderer>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spDeferredRenderer::Render()
  {
    spRenderContext* pRenderContext = GetSceneContext()->GetRenderContext();

    const auto cl = pRenderContext->GetCommandList();
    const spRenderView* pRenderView = pRenderContext->GetExtractionData().m_pRenderView;

    const auto& visibleObjects = pRenderContext->GetExtractionData().m_pRenderView->GetVisibleRenderObjects();

    cl->PushDebugGroup("Deferred Renderer");
    {
      // FIXME: Temporary hack to have something displayed on screen
      const auto fb = cl->PushRestoreFramebuffer(nullptr);

      // Light Clusters Pass
      {
      }

      // Geometry Pass
      {
        // Depth Pre-pass
        {
        }

        // Static Opaque/G-Buffer Pass
        pRenderContext->GetExtractionData().m_pRenderStage = m_pStaticOpaqueRenderStage.Borrow();
        {
          ezDynamicArray<spRenderObject*> opaqueObjects;

          // Filter
          m_pStaticOpaqueRenderStage->Filter(pRenderView, visibleObjects.GetArrayPtr(), &opaqueObjects, nullptr);

          // Sort
          m_pStaticOpaqueRenderStage->Sort(pRenderView, opaqueObjects, opaqueObjects);

          const ezUInt32 uiCurrentStaticPassHash = ezHashingUtils::CombineHashValues32(
            ezHashingUtils::xxHash32(&pRenderView->GetViewMatrix(), sizeof(ezMat4)),
            ezHashingUtils::xxHash32(&pRenderView->GetProjectionMatrix(), sizeof(ezMat4)));

          // TODO: Add a check to see if the render view has changed since the last frame
          if (!opaqueObjects.IsEmpty())
//          if (!opaqueObjects.IsEmpty() && m_uiLastStaticPassHash != uiCurrentStaticPassHash)
          {
            auto pushRestore = cl->PushRestoreFramebuffer(m_pStaticOpaqueRenderStage->GetOutputFramebuffer(pRenderView));

            const ezRectU32 viewport = pRenderView->GetViewport();
            const RHI::spViewport vp(viewport.x, viewport.y, viewport.width, viewport.height, 0.0f, 1.0f);
            cl->SetViewport(0, vp);

            // Clear
            cl->ClearColorTarget(0, ezColor::Black);
            cl->ClearDepthStencilTarget(1.0f, 0);

            // Draw
            m_pStaticOpaqueRenderStage->Draw(pRenderContext, opaqueObjects);

            m_uiLastStaticPassHash = uiCurrentStaticPassHash;
          }
        }

        // Dynamic Opaque/G-Buffer Pass
        {
        }
      }

      // Light Culling Pass
      {
      }

      // Lightning Pass
      {
      }

      // Particles Pass
      {
      }

      // Skybox Pass
      {
      }

      // Alpha Blending / Transparent Pass
      {
      }

      // FIXME: Temporary hack to have something displayed on screen
      cl->CopyTexture(
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(m_pStaticOpaqueRenderStage->GetOutputFramebuffer(pRenderView)->GetColorTargets()[0]),
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(fb.GetPreviousFramebuffer()->GetColorTargets()[0]));
    }
    cl->PopDebugGroup();
  }

  void spDeferredRenderer::Prepare()
  {
    auto& renderContextData = GetSceneContext()->GetRenderContext()->GetExtractionData();

    renderContextData.m_pRenderStage = m_pStaticOpaqueRenderStage.Borrow();
    m_pStaticOpaqueRenderStage->CreateOutputFramebuffer(renderContextData.m_pRenderView);
  }

  void spDeferredRenderer::Initialize(const spSceneContext* pSceneContext)
  {
    SUPER::Initialize(pSceneContext);
  }

  spDeferredRenderer::spDeferredRenderer()
    : SUPER("DeferredRenderer")
  {
    m_pStaticOpaqueRenderStage = EZ_DEFAULT_NEW(spOpaqueRenderStage);
  }

  spDeferredRenderer::~spDeferredRenderer()
  {
    m_pStaticOpaqueRenderStage.Clear();
  }
} // namespace RPI
