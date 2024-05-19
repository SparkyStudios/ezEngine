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

#include <RPI/Renderers/ClearRenderer.h>
#include <RPI/Scene/SceneContext.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_BITFLAGS(RPI::spClearRenderer::spClearFlags, 1)
  EZ_ENUM_CONSTANT(RPI::spClearRenderer::spClearFlags::Color),
  EZ_ENUM_CONSTANT(RPI::spClearRenderer::spClearFlags::Depth),
EZ_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

namespace RPI
{
  typedef ezRTTIDefaultAllocator<spClearRenderer, RHI::spDeviceAllocatorWrapper> spRTTIClearRendererAllocator;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spClearRenderer, 1, spRTTIClearRendererAllocator)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_BITFLAGS_MEMBER_PROPERTY("ClearFlags", spClearRenderer::spClearFlags, m_eClearFlags)->AddAttributes(new ezDefaultValueAttribute(spClearRenderer::spClearFlags::Default)),
      EZ_MEMBER_PROPERTY("Color", m_ClearColor)->AddAttributes(new ezDefaultValueAttribute(ezColor::SkyBlue)),
      EZ_MEMBER_PROPERTY("DepthValue", m_fClearDepth)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 1.0f)),
      EZ_MEMBER_PROPERTY("StencilValue", m_uiClearStencil)->AddAttributes(new ezDefaultValueAttribute(0), new ezClampValueAttribute(0, 255)),
    }
    EZ_END_PROPERTIES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spClearRenderer::Render()
  {
    const auto cl = GetSceneContext()->GetRenderContext()->GetCommandList();

    cl->PushDebugGroup("Clear Renderer");
    {
      if (m_eClearFlags.IsSet(spClearFlags::Color))
        cl->ClearColorTarget(0, m_ClearColor);

      if (m_eClearFlags.IsSet(spClearFlags::Depth))
        cl->ClearDepthStencilTarget(m_fClearDepth, m_uiClearStencil);
    }
    cl->PopDebugGroup();
  }

  void spClearRenderer::Prepare()
  {
    // noop
  }

  spClearRenderer::spClearRenderer()
    : spRenderer("ClearRenderer")
  {
  }

} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Renderers_ClearRenderer);
