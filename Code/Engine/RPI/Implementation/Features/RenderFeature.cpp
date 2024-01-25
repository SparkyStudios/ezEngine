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
#include <RPI/Features/RenderFeature.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderFeature, 1, ezRTTINoAllocator)
  {
    flags.Add(ezTypeFlags::Abstract);

    EZ_BEGIN_PROPERTIES
    {
      EZ_MEMBER_PROPERTY("Active", m_bIsActive)->AddAttributes(new ezDefaultValueAttribute(true)),
      EZ_ACCESSOR_PROPERTY("Name", GetName, SetName),
    }
    EZ_END_PROPERTIES;

    EZ_BEGIN_ATTRIBUTES
    {
      new ezColorAttribute(ezColorScheme::DarkUI(ezColorScheme::Red)),
      new ezCategoryAttribute("Features"),
    }
    EZ_END_ATTRIBUTES;
  }
  EZ_END_DYNAMIC_REFLECTED_TYPE
  // clang-format on

  spRenderFeature::spRenderFeature(ezInternal::NewInstance<spRenderFeatureExtractor> pExtractor)
    : m_pExtractor(pExtractor)
  {
  }

  void spRenderFeature::Initialize(const spRenderContext* pRenderContext)
  {
    EZ_ASSERT_DEV(pRenderContext != nullptr, "Render context must not be nullptr.");

    if (m_pRenderContext != nullptr)
      Deinitialize();

    m_pRenderContext = pRenderContext;
    m_bIsInitialized = true;

    OnInitialize();

    spRenderSystem::GetCollectEvent().AddEventHandler(ezMakeDelegate(&spRenderFeature::OnRenderSystemCollectEvent, this));
  }

  void spRenderFeature::Deinitialize()
  {
    spRenderSystem::GetCollectEvent().RemoveEventHandler(ezMakeDelegate(&spRenderFeature::OnRenderSystemCollectEvent, this));

    OnDeinitialize();

    m_bIsInitialized = false;
    m_pRenderContext = nullptr;
  }

  void spRenderFeature::Extract(const spRenderContext* pRenderContext, const spRenderView* pRenderView) const
  {
    m_pExtractor->Extract(pRenderContext, pRenderView);
  }

  bool spRenderFeature::TryAddRenderObject(spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "Render object must not be nullptr.");

    pRenderObject->m_pRenderFeature = this;
    pRenderObject->m_RenderFeatureReference = spRenderNodeReference(m_RenderObjects.GetCount());

    m_RenderObjects.PushBack(pRenderObject);

    return true;
  }

  void spRenderFeature::RemoveRenderObject(spRenderObject* pRenderObject)
  {
    EZ_ASSERT_DEV(pRenderObject != nullptr, "Render object must not be nullptr");
    EZ_ASSERT_DEV(pRenderObject->m_pRenderFeature == this, "Render object must match the render feature");

    const auto iObjectRef = pRenderObject->m_RenderFeatureReference.GetRef();

    pRenderObject->m_RenderFeatureReference = spRenderNodeReference::MakeInvalid();

    m_RenderObjects.RemoveAtAndSwap(iObjectRef);

    if (iObjectRef < m_RenderObjects.GetCount())
      m_RenderObjects[iObjectRef]->m_RenderFeatureReference = spRenderNodeReference(iObjectRef);

    pRenderObject->m_pRenderFeature = nullptr;
  }

  bool spRenderFeature::TryAddRenderComponent(spRenderComponent* pRenderComponent)
  {
    EZ_ASSERT_DEV(pRenderComponent != nullptr, "Render component must not be nullptr.");

    pRenderComponent->m_pRenderFeature = this;
    pRenderComponent->m_RenderFeatureReference = spRenderNodeReference(m_RenderComponents.GetCount());

    m_RenderComponents.PushBack(pRenderComponent);

    return true;
  }

  void spRenderFeature::RemoveRenderComponent(spRenderComponent* pRenderComponent)
  {
    EZ_ASSERT_DEV(pRenderComponent != nullptr, "Render component must not be nullptr.");
    EZ_ASSERT_DEV(pRenderComponent->m_pRenderFeature == this, "Render component must match the render feature");

    const auto iComponentRef = pRenderComponent->m_RenderFeatureReference.GetRef();

    pRenderComponent->m_RenderFeatureReference = spRenderNodeReference::MakeInvalid();

    m_RenderComponents.RemoveAtAndSwap(iComponentRef);

    if (iComponentRef < m_RenderComponents.GetCount())
      m_RenderComponents[iComponentRef]->m_RenderFeatureReference = spRenderNodeReference(iComponentRef);

    pRenderComponent->m_pRenderFeature = nullptr;
  }

  void spRenderFeature::OnRenderSystemCollectEvent(const spRenderSystemCollectEvent& event)
  {
    if (!m_bIsActive || m_pExtractor == nullptr)
      return;

    auto* pRenderSystem = ezSingletonRegistry::GetRequiredSingletonInstance<spRenderSystem>();
    pRenderSystem->GetRenderFeatureCollector().Add(this);
  }

  void spRenderFeature::OnInitialize()
  {
  }

  void spRenderFeature::OnDeinitialize()
  {
  }
} // namespace RPI
