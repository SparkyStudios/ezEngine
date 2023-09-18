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

#include <RPI/Core.h>
#include <RPI/Core/RenderContext.h>

namespace RPI
{
  class spRenderPipeline;
  class spRenderPass;

  /// \brief An event triggered before the start and after the end of a \a spRenderPass.
  struct spRenderPassEvent
  {
    enum class Type
    {
      BeforePass,
      AfterPass,

      Unknown,
    };

    Type m_Type{Type::Unknown};
    spRenderPipeline* m_pPipeline{nullptr};
    spRenderPass* m_pPass{nullptr};
    spRenderContext* m_pContext{nullptr};
  };

  class SP_RPI_DLL spRenderPipeline : public ezRefCounted
  {
    EZ_DISALLOW_COPY_AND_ASSIGN(spRenderPipeline);

  public:
    explicit spRenderPipeline(spRenderGraphResourcesTable&& resources);
    ~spRenderPipeline() override;

    EZ_NODISCARD EZ_ALWAYS_INLINE const spRenderGraphResourcesTable& GetResources() const { return m_PipelineResources; }

    void Execute(spRenderContext* pContext);

    void BeginPass(spRenderPass* pPass, spRenderContext* pContext);
    void ExecutePass(spRenderPass* pPass, spRenderContext* pContext);
    void EndPass(spRenderPass* pPass, spRenderContext* pContext);

    void AddPass(ezHashedString sName, ezUniquePtr<spRenderPass>&& pPass);
    void RemovePass(ezHashedString sName);

    bool TryGetPass(ezHashedString sName, spRenderPass*& out_pPass);
    spRenderPass* GetPass(ezHashedString sName);

    void CleanUp();

  private:
    ezEvent<const spRenderPassEvent&, ezMutex> m_PassEvents;

    spRenderGraphResourcesTable m_PipelineResources;

    ezHashTable<ezHashedString, ezUniquePtr<spRenderPass>> m_Passes;
    ezDynamicArray<ezHashedString> m_OrderedPasses;
  };
} // namespace RPI