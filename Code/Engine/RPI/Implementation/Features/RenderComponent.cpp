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

#include <RPI/Core/RenderSystem.h>
#include <RPI/Features/RenderComponent.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(spRenderComponent, 1)
  EZ_END_ABSTRACT_COMPONENT_TYPE;
  // clang-format on

  void spRenderComponent::Deinitialize()
  {
    ezComponent::Deinitialize();
  }

  void spRenderComponent::OnActivated()
  {
    ezComponent::OnActivated();

    for (auto&& pFeature : m_pRenderSystem->GetRenderFeatureCollector())
    {
      if (pFeature->IsInstanceOf(m_pRenderFeatureType) && pFeature->TryAddRenderComponent(this))
      {
        m_bHasFeatureAvailable = true;
        break;
      }
    }
  }

  void spRenderComponent::OnDeactivated()
  {
    ezComponent::OnDeactivated();

    if (!m_bHasFeatureAvailable)
      return;

    m_pRenderFeature->RemoveRenderComponent(this);
  }

  spRenderComponent::spRenderComponent(ezRTTI* pRenderFeatureType)
    : m_pRenderFeatureType(pRenderFeatureType)
  {
    m_pRenderSystem = spRenderSystem::GetSingleton();
  }

  spRenderComponent::~spRenderComponent()
  {
    m_pRenderFeatureType = nullptr;
    m_pRenderSystem = nullptr;
    m_pRenderFeature = nullptr;
    m_bHasFeatureAvailable = false;
  }
} // namespace RPI
