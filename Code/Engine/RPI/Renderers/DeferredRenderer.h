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

#include <RPI/Core/Renderer.h>

namespace RPI
{
  class spOpaqueRenderStage;

  class SP_RPI_DLL spDeferredRenderer : public spRenderer
  {
    EZ_ADD_DYNAMIC_REFLECTION(spDeferredRenderer, spRenderer);

    // spRenderer

  public:
    void Render() override;
    void Prepare() override;
    void Initialize(const spSceneContext* pSceneContext) override;

    // spDeferredRenderer

  public:
    spDeferredRenderer();
    ~spDeferredRenderer() override;

  private:
    ezUniquePtr<spOpaqueRenderStage> m_pOpaqueRenderStage{nullptr};
  };
} // namespace RPI
