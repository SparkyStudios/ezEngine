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
#include <RPI/Scene/SceneContext.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(spRenderComponent, 1)
  {
    EZ_BEGIN_ATTRIBUTES
    {
      new ezCategoryAttribute("Rendering"),
    }
    EZ_END_ATTRIBUTES;

    EZ_BEGIN_MESSAGEHANDLERS
    {
      EZ_MESSAGE_HANDLER(ezMsgUpdateLocalBounds, OnUpdateLocalBounds)
    }
    EZ_END_MESSAGEHANDLERS;
  }
  EZ_END_ABSTRACT_COMPONENT_TYPE;
  // clang-format on

  void spRenderComponent::Deinitialize()
  {
    SUPER::Deinitialize();
  }

  void spRenderComponent::OnActivated()
  {
    SUPER::OnActivated();

    spSceneContext* pSceneContext = m_pRenderSystem->GetSceneContextFromWorld(GetWorld());
    if (pSceneContext == nullptr)
      return;

    for (auto&& pFeature : pSceneContext->GetRenderFeatureCollector())
    {
      if (pFeature->IsInstanceOf(m_pRenderFeatureType) && pFeature->TryAddRenderComponent(this))
      {
        m_bHasFeatureAvailable = true;
        break;
      }
    }

    TriggerLocalBoundsUpdate();
  }

  void spRenderComponent::OnDeactivated()
  {
    SUPER::OnDeactivated();

    GetOwner()->UpdateLocalBounds();

    if (!m_bHasFeatureAvailable)
      return;

    m_pRenderFeature->RemoveRenderComponent(this);
  }

  spRenderComponent::spRenderComponent(const ezRTTI* pRenderFeatureType)
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

  void spRenderComponent::TriggerLocalBoundsUpdate()
  {
    if (IsActiveAndInitialized())
    {
      GetOwner()->UpdateLocalBounds();
    }
  }

  void spRenderComponent::OnUpdateLocalBounds(ezMsgUpdateLocalBounds& msg)
  {
    ezBoundingBoxSphere bounds = ezBoundingBoxSphere::MakeInvalid();

    if (bool bAlwaysVisible = false; GetLocalBounds(bounds, bAlwaysVisible).Succeeded())
    {
      const ezSpatialData::Category category = GetOwner()->IsDynamic() ? ezDefaultSpatialDataCategories::RenderDynamic : ezDefaultSpatialDataCategories::RenderStatic;

      if (bounds.IsValid())
      {
        msg.AddBounds(bounds, category);
      }

      if (bAlwaysVisible)
      {
        msg.SetAlwaysVisible(category);
      }
    }
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Features_RenderComponent);
