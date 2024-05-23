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

#include <RPI/Core/RenderStage.h>

namespace RPI
{
  /// \brief A render stage that renders opaque objects. Mostly used by the forward renderer.
  class SP_RPI_DLL spOpaqueRenderStage : public spRenderStage
  {
    EZ_ADD_DYNAMIC_REFLECTION(spOpaqueRenderStage, spRenderStage);

    // spRenderStage

  public:
    void Filter(const spRenderView& renderView, const ezArrayPtr<spRenderObject>& renderObjects, ezArrayPtr<spRenderObject>& out_filteredRenderObjects, ezArrayPtr<spRenderObject>& out_unfilteredRenderObjects) override;

    void Sort(const spRenderView& renderView, const ezArrayPtr<spRenderObject>& renderObjects, ezArrayPtr<spRenderObject>& out_sortedRenderObjects) override;

    bool IsActiveForRenderObject(const spRenderObject* pRenderObject) const override;

    RHI::spRenderingState GetRenderingState(const spRenderObject* pRenderObject) const override;

    void CreateOutputFramebuffer(const spRenderView* pRenderView) override;

    // spOpaqueRenderStage

  public:
    spOpaqueRenderStage();
    ~spOpaqueRenderStage() override = default;

  private:
    ezArrayMap<const spRenderView*, ezSharedPtr<RHI::spTexture>> m_DepthStencilTextures;
    ezArrayMap<const spRenderView*, ezSharedPtr<RHI::spTexture>> m_ColorTextures;
  };
} // namespace RPI
