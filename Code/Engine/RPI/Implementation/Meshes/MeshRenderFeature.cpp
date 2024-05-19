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

#include <RPI/Meshes/MeshRenderFeature.h>
#include <RPI/Meshes/MeshRenderFeatureExtractor.h>
#include <RPI/Meshes/MeshRenderObject.h>

namespace RPI
{
  typedef ezRTTIDefaultAllocator<spMeshRenderFeature, RHI::spDeviceAllocatorWrapper> spRTTIMeshRenderFeatureAllocator;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMeshRenderFeature, 1, spRTTIMeshRenderFeatureAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spMeshRenderFeature::GetSupportedRenderObjectTypes(ezHybridArray<const ezRTTI*, 8>& out_Types) const
  {
    out_Types.SetCount(1);
    out_Types.PushBack(ezGetStaticRTTI<spMeshRenderObject>());
  }

  void spMeshRenderFeature::Render(const spRenderContext* pRenderingContext) const
  {
    auto cl = pRenderingContext->GetCommandList();

    ezSharedPtr<RHI::spScopeProfiler> pScopeProfiler;

    cl->PushProfileScope("spMeshRenderFeature");
    {
      cl->ClearColorTarget(0, ezColor::Maroon);
    }
    cl->PopProfileScope(pScopeProfiler);
  }

  spMeshRenderFeature::spMeshRenderFeature()
    : spRenderFeature(EZ_NEW(RHI::spDeviceAllocatorWrapper::GetAllocator(), spMeshRenderFeatureExtractor))
  {
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Meshes_MeshRenderFeature);
