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

#include "RPI/Camera/Camera.h"
#include "RPI/Stages/OpaqueRenderStage.h"


#include <RPI/RPIPCH.h>

#include <RPI/Renderers/CameraRenderer.h>
#include <RPI/Scene/SceneContext.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spCameraRenderer, 1, ezRTTIDefaultAllocator<spCameraRenderer>)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_ACCESSOR_PROPERTY("Slot", GetCameraSlot, SetCameraSlot)->AddAttributes(new ezDynamicStringEnumAttribute("CameraSlot")),
    }
    EZ_END_PROPERTIES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  static spOpaqueRenderStage* s_pOpaqueRenderStage;

  void spCameraRenderer::Render()
  {
    spCamera* pCamera = spRenderSystem::GetSingleton()->GetCompositor()->GetCameraBySlot(m_hCameraSlot);
    if (pCamera == nullptr)
      return; // No camera found for the given slot, nothing to do.

    const spRenderContext* pRenderContext = GetSceneContext()->GetRenderContext();
    const RHI::spDevice* pDevice = pRenderContext->GetDevice();

    const auto cl = pRenderContext->GetCommandList();
    const auto pFramebuffer = pDevice->GetMainSwapchain()->GetFramebuffer();

    const spRenderView* pRenderView = pRenderContext->GetExtractionData().m_pRenderView;
    const spRenderStage* pRenderStage = pRenderContext->GetExtractionData().m_pRenderStage;

    cl->PushDebugGroup("Camera Renderer");
    {
      auto clPushRestore = cl->PushRestoreFramebuffer(pRenderStage->GetOutputFramebuffer(pRenderView));
      cl->SetFullViewport(0);
      cl->SetFullScissorRect(0);

      const ezRectU32 viewport = pRenderView->GetViewport();
      const RHI::spViewport vp(viewport.x, viewport.y, viewport.width, viewport.height, 0.0f, 1.0f);
      cl->SetViewport(0, vp);

      SUPER::Render();

      const auto& visibleObjects = GetSceneContext()->GetRenderContext()->GetExtractionData().m_pRenderView->GetVisibleRenderObjects();
      for (const auto& pObject : visibleObjects)
        pObject->Draw(GetSceneContext()->GetRenderContext());

      cl->CopyTexture(
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(s_pOpaqueRenderStage->GetOutputFramebuffer(pCamera->GetRenderView())->GetColorTargets()[0]),
        cl->GetDevice()->GetResourceManager()->GetResource<RHI::spTexture>(pFramebuffer->GetColorTargets()[0]));
    }
    cl->PopDebugGroup();
  }

  void spCameraRenderer::Prepare()
  {
    spCamera* pCamera = spRenderSystem::GetSingleton()->GetCompositor()->GetCameraBySlot(m_hCameraSlot);

    GetSceneContext()->GetRenderContext()->GetExtractionData().m_pRenderView = pCamera->GetRenderView();
    GetSceneContext()->GetRenderContext()->GetExtractionData().m_pRenderStage = s_pOpaqueRenderStage;

    s_pOpaqueRenderStage->CreateOutputFramebuffer(pCamera->GetRenderView());

    if (m_bCameraChanged)
    {
      m_bCameraChanged = false;
    }

    SUPER::Prepare();
  }

  void spCameraRenderer::Initialize(const spSceneContext* pSceneContext)
  {
    SUPER::Initialize(pSceneContext);
  }

  spCameraRenderer::spCameraRenderer()
    : spParentRenderer("CameraRenderer")
  {
    s_pOpaqueRenderStage = EZ_DEFAULT_NEW(spOpaqueRenderStage);
  }

  spCameraRenderer::~spCameraRenderer()
  {
    // m_pRenderTarget.Clear();
    EZ_DEFAULT_DELETE(s_pOpaqueRenderStage);
  }

  void spCameraRenderer::SetCameraSlot(const char* szCameraSlot)
  {
    if (m_sCameraSlotName == ezTempHashedString(szCameraSlot))
      return;

    m_hCameraSlot = spRenderSystem::GetSingleton()->GetCompositor()->GetCameraSlotByName(szCameraSlot);
    m_sCameraSlotName = szCameraSlot;
    m_bCameraChanged = true;
  }

  const char* spCameraRenderer::GetCameraSlot() const
  {
    return spRenderSystem::GetSingleton()->GetCompositor()->GetCameraSlot(m_hCameraSlot)->GetName().GetStartPointer();
  }

  void spCameraRenderer::OnInitialize()
  {
  }
} // namespace RPI
