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
        pRenderContext->GetExtractionData().m_pRenderStage = m_pOpaqueRenderStage.Borrow();
        {
          auto pushRestore = cl->PushRestoreFramebuffer(m_pOpaqueRenderStage->GetOutputFramebuffer(pRenderView));

          const ezRectU32 viewport = pRenderView->GetViewport();
          const RHI::spViewport vp(viewport.x, viewport.y, viewport.width, viewport.height, 0.0f, 1.0f);
          cl->SetViewport(0, vp);

          // Clear
          cl->InsertDebugMarker("Clear Opaque G-Buffer Start");
          cl->ClearColorTarget(0, ezColor::SkyBlue);
          cl->ClearDepthStencilTarget(1.0f, 0);
          cl->InsertDebugMarker("Clear Opaque G-Buffer End");

          const auto& visibleObjects = GetSceneContext()->GetRenderContext()->GetExtractionData().m_pRenderView->GetVisibleRenderObjects();

          // Filter
          ezDynamicArray<spRenderObject*> opaqueObjects;
          m_pOpaqueRenderStage->Filter(pRenderView, visibleObjects.GetArrayPtr(), &opaqueObjects, nullptr);

          // Sort
          ezDynamicArray<spRenderObject*> sortedObjects;
          m_pOpaqueRenderStage->Sort(pRenderView, opaqueObjects, sortedObjects);

          // Draw
          for (const auto& pObject : sortedObjects)
            pObject->Draw(GetSceneContext()->GetRenderContext());
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
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(m_pOpaqueRenderStage->GetOutputFramebuffer(pRenderView)->GetColorTargets()[0]),
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(fb.GetPreviousFramebuffer()->GetColorTargets()[0]));
    }
    cl->PopDebugGroup();
  }

  void spDeferredRenderer::Prepare()
  {
    auto& renderContextData = GetSceneContext()->GetRenderContext()->GetExtractionData();

    renderContextData.m_pRenderStage = m_pOpaqueRenderStage.Borrow();
    m_pOpaqueRenderStage->CreateOutputFramebuffer(renderContextData.m_pRenderView);
  }

  void spDeferredRenderer::Initialize(const spSceneContext* pSceneContext)
  {
    SUPER::Initialize(pSceneContext);
  }

  spDeferredRenderer::spDeferredRenderer()
    : SUPER("DeferredRenderer")
  {
    m_pOpaqueRenderStage = EZ_DEFAULT_NEW(spOpaqueRenderStage);
  }

  spDeferredRenderer::~spDeferredRenderer()
  {
    m_pOpaqueRenderStage.Clear();
  }
} // namespace RPI
