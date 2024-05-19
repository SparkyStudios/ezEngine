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

#include <RPI/Core/Renderer.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderer, 1, ezRTTINoAllocator)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_MEMBER_PROPERTY("Name", m_sName),
      EZ_MEMBER_PROPERTY("Enabled", m_bEnabled),
      EZ_ACCESSOR_PROPERTY_READ_ONLY("Initialized", IsInitialized),
    }
    EZ_END_PROPERTIES;

    EZ_BEGIN_ATTRIBUTES
    {
      new ezCategoryAttribute("Renderers"),
    }
    EZ_END_ATTRIBUTES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spParentRenderer, 1, ezRTTINoAllocator)
  {
    EZ_BEGIN_PROPERTIES
    {
      EZ_ACCESSOR_PROPERTY("Child", GetChildRenderer, SetChildRenderer),
    }
    EZ_END_PROPERTIES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spRenderer::spRenderer(ezStringView sName)
    : m_sName(sName)
  {
  }

  void spRenderer::Initialize(const spSceneContext* pSceneContext)
  {
    if (m_bInitialized)
      return;

    m_pSceneContext = pSceneContext;
    m_bInitialized = true;

    OnInitialize();
  }

  void spRenderer::OnInitialize()
  {
  }

  void spParentRenderer::Render()
  {
    if (m_pChildRenderer == nullptr)
      return;

    m_pChildRenderer->Render();
  }

  void spParentRenderer::Prepare()
  {
    if (m_pChildRenderer == nullptr)
      return;

    m_pChildRenderer->Prepare();
  }

  void spParentRenderer::Initialize(const spSceneContext* pSceneContext)
  {
    SUPER::Initialize(pSceneContext);

    if (m_pChildRenderer == nullptr)
      return;

    m_pChildRenderer->Initialize(pSceneContext);
  }

  spParentRenderer::spParentRenderer(ezStringView sName)
    : spRenderer(sName)
  {
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Core_Renderer);
