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

#include <RPI/Camera/Camera.h>
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

  void spCameraRenderer::Render()
  {
    spCamera* pCamera = ResolveCamera();
    if (pCamera == nullptr)
      return; // No camera found for the given slot, nothing to do.

    const spRenderContext* pRenderContext = GetSceneContext()->GetRenderContext();
    const RHI::spDevice* pDevice = GetSceneContext()->GetDevice();

    const auto cl = pRenderContext->GetCommandList();
    const auto pFramebuffer = pDevice->GetMainSwapchain()->GetFramebuffer();

    cl->PushDebugGroup("Camera Renderer");
    {
      SUPER::Render();
    }
    cl->PopDebugGroup();
  }

  void spCameraRenderer::Prepare()
  {
    spCamera* pCamera = ResolveCamera();
    if (pCamera == nullptr)
      return; // No camera found for the given slot, nothing to do.

    const auto pCompositor = spRenderSystem::GetSingleton()->GetCompositor();
    GetSceneContext()->GetRenderContext()->GetExtractionData().m_pRenderView = pCamera->GetRenderView();

    if (m_bCameraChanged)
    {
      m_bCameraChanged = false;

      const ezRectU32 renderSize = pCompositor->GetRenderSize();
      pCamera->GetRenderView()->SetViewport(renderSize);
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
  }

  spCameraRenderer::~spCameraRenderer()
  {
    m_hCameraSlot.Invalidate();
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

  spCamera* spCameraRenderer::ResolveCamera() const
  {
    if (m_hCameraSlot.IsInvalidated())
      return nullptr;

    return spRenderSystem::GetSingleton()->GetCompositor()->GetCameraBySlot(m_hCameraSlot);
  }
} // namespace RPI
