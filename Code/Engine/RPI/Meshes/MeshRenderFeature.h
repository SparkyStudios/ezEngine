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
#include <RPI/Meshes/MeshRenderFeatureExtractor.h>

namespace RPI
{
  class SP_RPI_DLL spMeshRenderFeature final : public spRenderFeature
  {
    EZ_ADD_DYNAMIC_REFLECTION(spMeshRenderFeature, spRenderFeature);

    // spRenderFeature

  public:
    void GetSupportedRenderObjectTypes(ezHybridArray<const ezRTTI*, 8>& out_Types) const override;
    void Render(const spRenderContext* pRenderingContext) const override;

    // spMeshRenderFeature

  public:
    spMeshRenderFeature();
    ~spMeshRenderFeature() override = default;

  private:

  };
} // namespace RPI
