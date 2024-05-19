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

#include <RPI/Core/RenderContext.h>
#include <RPI/Graph/RenderGraph.h>

namespace RPI
{
  class SP_RPI_DLL spRenderPass
  {
    EZ_DISALLOW_COPY_AND_ASSIGN(spRenderPass);

  public:
    spRenderPass() = default;
    virtual ~spRenderPass() = default;

    template <typename T>
    EZ_ALWAYS_INLINE void SetData(const T& data)
    {
      m_PassData = data;
    }

    virtual void Execute(const spRenderGraphResourcesTable& resources, const spRenderContext* context) = 0;
    virtual void CleanUp(const spRenderGraphResourcesTable& resources) = 0;

  protected:
    ezVariant m_PassData;
  };

  class SP_RPI_DLL spCallbackRenderPass : public spRenderPass
  {

  public:
    typedef ezDelegate<void(const spRenderGraphResourcesTable&, const spRenderContext*, ezVariant&)> ExecuteCallback;
    typedef ezDelegate<void(const spRenderGraphResourcesTable&, ezVariant&)> CleanUpCallback;

    spCallbackRenderPass(ExecuteCallback executeCallback, CleanUpCallback cleanUpCallback);
    ~spCallbackRenderPass() override = default;

    void Execute(const spRenderGraphResourcesTable& resources, const spRenderContext* context) override;
    void CleanUp(const spRenderGraphResourcesTable& resources) override;

  private:
    ExecuteCallback m_ExecuteCallback;
    CleanUpCallback m_CleanUpCallback;
  };
} // namespace RPI
