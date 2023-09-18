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

#include <RPI/Pipeline/RenderPass.h>

namespace RPI
{
  spRenderPass::spRenderPass(ExecuteCallback executeCallback, CleanUpCallback cleanUpCallback)
    : m_ExecuteCallback(std::move(executeCallback))
    , m_CleanUpCallback(std::move(cleanUpCallback))
  {
  }

  void spRenderPass::Execute(const spRenderGraphResourcesTable& resources, spRenderContext* context)
  {
    m_ExecuteCallback(resources, context, m_PassData);
  }

  void spRenderPass::CleanUp(const spRenderGraphResourcesTable& resources)
  {
    m_CleanUpCallback(resources, m_PassData);
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Pipeline_RenderPass);
