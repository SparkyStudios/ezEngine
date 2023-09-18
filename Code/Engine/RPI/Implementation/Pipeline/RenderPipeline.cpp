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
#include <RPI/Pipeline/RenderPipeline.h>

namespace RPI
{
  spRenderPipeline::spRenderPipeline(spRenderGraphResourcesTable&& resources)
    : m_PipelineResources(resources)
  {
  }

  spRenderPipeline::~spRenderPipeline()
  {
    for (const auto& pass : m_OrderedPasses)
      RemovePass(pass);
  }

  void spRenderPipeline::Execute(spRenderContext* pContext)
  {
    for (const auto& pass : m_OrderedPasses)
    {
      const ezUniquePtr<spRenderPass>* pPass = m_Passes.GetValue(pass);

      BeginPass(pPass->Borrow(), pContext);
      ExecutePass(pPass->Borrow(), pContext);
      EndPass(pPass->Borrow(), pContext);
    }
  }

  void spRenderPipeline::BeginPass(spRenderPass* pPass, spRenderContext* pContext)
  {
    m_PassEvents.Broadcast({spRenderPassEvent::Type::BeforePass, this, pPass, pContext});
  }

  void spRenderPipeline::ExecutePass(spRenderPass* pPass, spRenderContext* pContext)
  {
    pPass->Execute(m_PipelineResources, pContext);
  }

  void spRenderPipeline::EndPass(spRenderPass* pPass, spRenderContext* pContext)
  {
    m_PassEvents.Broadcast({spRenderPassEvent::Type::AfterPass, this, pPass, pContext});
  }

  void spRenderPipeline::AddPass(ezHashedString sName, ezUniquePtr<spRenderPass>&& pPass)
  {
    m_Passes.Insert(sName, std::move(pPass));
    m_OrderedPasses.PushBack(sName);
  }

  void spRenderPipeline::RemovePass(ezHashedString sName)
  {
    m_Passes.Remove(sName);
    m_OrderedPasses.RemoveAndCopy(sName);
  }

  void spRenderPipeline::CleanUp()
  {
    for (const auto& pass : m_OrderedPasses)
    {
      const ezUniquePtr<spRenderPass>* pPass = m_Passes.GetValue(pass);
      (*pPass)->CleanUp(m_PipelineResources);
    }
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Pipeline_RenderPipeline);
