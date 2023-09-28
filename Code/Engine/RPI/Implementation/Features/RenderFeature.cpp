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
    EZ_BEGIN_PROPERTIES
    {
      EZ_MEMBER_PROPERTY("Active", m_bIsActive)->AddAttributes(new ezDefaultValueAttribute(true)),
      EZ_ACCESSOR_PROPERTY("Name", GetName, SetName),
    }
    EZ_END_PROPERTIES;

    EZ_BEGIN_ATTRIBUTES
    {
      new ezColorAttribute(ezColorScheme::DarkUI(ezColorScheme::Red)),
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

  void spRenderFeature::OnRenderSystemCollectEvent(const spRenderSystemCollectEvent& event)
  {
    if (!m_bIsActive || m_pExtractor == nullptr)
      return;

    auto* pRenderSystem = ezSingletonRegistry::GetRequiredSingletonInstance<spRenderSystem>();
    pRenderSystem->GetRenderFeatureExtractorCollector().Add(m_pExtractor.Borrow());
  }

  void spRenderFeature::OnInitialize()
  {
  }

  void spRenderFeature::OnDeinitialize()
  {
  }
} // namespace RPI
