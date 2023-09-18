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

#include <RPI/Pipeline/Passes/ClusteredDeferredClustersComputeRenderPass.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(RPI::spClusteredDeferredClustersComputeRenderPass::Data, ezNoBase, 1, ezRTTIDefaultAllocator<RPI::spClusteredDeferredClustersComputeRenderPass::Data>)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace RPI
{
  spClusteredDeferredClustersComputeRenderPass::spClusteredDeferredClustersComputeRenderPass(Data&& passData)
    : spRenderPass({}, {})
  {
    SetData(passData);
  }

  void spClusteredDeferredClustersComputeRenderPass::Execute(const spRenderGraphResourcesTable& resources, spRenderContext* context)
  {
    auto const cl = context->GetCommandList();
  }

  void spClusteredDeferredClustersComputeRenderPass::CleanUp(const spRenderGraphResourcesTable& resources)
  {
    spRenderPass::CleanUp(resources);
  }
} // namespace RPI