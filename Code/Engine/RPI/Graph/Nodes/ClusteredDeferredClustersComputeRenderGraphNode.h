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

#include <RPI/Graph/RenderGraph.h>
#include <RPI/Pipeline/Passes/ClusteredDeferredClustersComputeRenderPass.h>

#include <RHI/Core.h>

namespace RPI
{
  class SP_RPI_DLL spClusteredDeferredClustersComputeRenderGraphNode final : public spRenderGraphNode
  {
    EZ_ADD_DYNAMIC_REFLECTION(spClusteredDeferredClustersComputeRenderGraphNode, spRenderGraphNode);

    // --- spRenderGraphNode

  public:
    ezResult Setup(spRenderGraphBuilder* pBuilder, const ezHashTable<ezHashedString, RHI::spResourceHandle>& resources) override;
    ezUniquePtr<spRenderPass> Compile(spRenderGraphBuilder* pBuilder) override;
    bool IsEnabled() const override;

    // --- spClusteredDeferredRenderGraphNode

  public:
    spClusteredDeferredClustersComputeRenderGraphNode();

  private:
    spClusteredDeferredClustersComputeRenderPass::Data m_PassData;
  };
} // namespace RPI
