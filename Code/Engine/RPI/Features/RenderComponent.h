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

#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Features/RenderFeature.h>

#include <Core/World/World.h>

namespace RPI
{
  /// \brief Base class for components that can render to the screen.
  class SP_RPI_DLL spRenderComponent : public ezComponent
  {
    friend class spRenderFeature;

    EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(spRenderComponent, ezComponent);

    // ezComponent

  protected:
    void Deinitialize() override;
    void OnActivated() override;
    void OnDeactivated() override;

    // spRenderComponent

  public:
    explicit spRenderComponent(ezRTTI* pRenderFeatureType);
    ~spRenderComponent() override;

  protected:
    spRenderFeature* m_pRenderFeature{nullptr};
    spRenderNodeReference m_RenderFeatureReference{spRenderNodeReference::MakeInvalid()};

    bool m_bHasFeatureAvailable{false};
    spRenderSystem* m_pRenderSystem{nullptr};

  private:
    ezRTTI* m_pRenderFeatureType{nullptr};
  };
} // namespace RPI
