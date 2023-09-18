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

#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Pipeline/RenderPass.h>

#include <RHI/Core.h>

namespace RPI
{
  class SP_RPI_DLL spClusteredDeferredClustersComputeRenderPass : public spRenderPass
  {
    // --- spRenderPass

  public:
    void Execute(const spRenderGraphResourcesTable& resources, spRenderContext* context) override;
    void CleanUp(const spRenderGraphResourcesTable& resources) override;

    // --- spClusteredRenderPass

  public:
    static constexpr ezUInt32 THREAD_GROUP_SIZE_X = 16;
    static constexpr ezUInt32 THREAD_GROUP_SIZE_Y = 9;
    static constexpr ezUInt32 THREAD_GROUP_SIZE_Z = 24;

    struct Data
    {
      EZ_DECLARE_POD_TYPE();
    };

    explicit spClusteredDeferredClustersComputeRenderPass(Data&& passData);

  private:
    ezSharedPtr<RHI::spComputePipeline> m_ComputePipeline;
    ezSharedPtr<RHI::spResourceSet> m_ResourceSet;
  };
} // namespace RPI

EZ_DECLARE_CUSTOM_VARIANT_TYPE(RPI::spClusteredDeferredClustersComputeRenderPass::Data);

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spClusteredDeferredClustersComputeRenderPass::Data);
