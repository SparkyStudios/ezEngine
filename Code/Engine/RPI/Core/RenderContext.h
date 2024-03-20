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

#include <RHI/CommandList.h>
#include <RHI/Device.h>

#include <RPI/Core/RenderContextData.h>

class ezWorld;

namespace RPI
{
  class SP_RPI_DLL spRenderContext
  {
  public:
    explicit spRenderContext(RHI::spDevice* pDevice);
    virtual ~spRenderContext();

    /// \brief Sets the current context's command list with an existing instance.
    /// \param pCommandList The new command list.
    void SetCommandList(ezSharedPtr<RHI::spCommandList> pCommandList);

    /// \brief Creates a new  context's command list from the given descriptor.
    /// \param description The command list descriptor.
    EZ_ALWAYS_INLINE void SetCommandList(const RHI::spCommandListDescription& description) { m_pCommandList = m_pDevice->GetResourceFactory()->CreateCommandList(description); }

    /// \brief Returns the current context's command list.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spCommandList> GetCommandList() const { return m_pCommandList; }

    /// \brief Gets the device used by this context.
    EZ_NODISCARD EZ_ALWAYS_INLINE RHI::spDevice* GetDevice() const { return m_pDevice; }

    /// \brief Marks the beginning of a render process.
    /// Render passes will be executed after this call and commands will be collected
    /// in the current command list.
    /// When the render process is finished, the command list will be executed by
    /// calling \a EndFrame.
    void BeginFrame();

    /// \brief Marks the end of a render process.
    /// This will execute all commands that were collected in the current context
    /// and reset the command list for the next render process.
    /// \param [in] pFence The \a spFence which will be signaled after this submission fully completes execution.
    void EndFrame(ezSharedPtr<RHI::spFence> pFence = nullptr);

    /// \brief Resets the current state of the \a spRenderContext.
    void Reset();

    EZ_NODISCARD const spRenderContextData& GetExtractionData() const;

    spRenderContextData& GetExtractionData();

  private:
    RHI::spDevice* m_pDevice{nullptr};

    ezSharedPtr<RHI::spCommandList> m_pCommandList{nullptr};
    ezSharedPtr<RHI::spBuffer> m_pRenderViewBuffer{nullptr};

    ezMutex m_CommandListLock;

    spRenderContextData m_Data[2];
  };
} // namespace RPI
