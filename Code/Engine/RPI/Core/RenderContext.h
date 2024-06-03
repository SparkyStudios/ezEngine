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
#include <RPI/Shaders/ConstantBuffer.h>
#include <RPI/Shaders/ShaderManager.h>
#include <RPI/Shaders/ShaderTypes.h>

class ezWorld;

namespace RPI
{
  class spSceneContext;

  struct alignas(16) spRenderFrameData
  {
    float m_fGlobalTime;
    float m_fCurrentTime;
    float m_fDeltaTime;
    ezUInt32 m_uiFrame;

    spShaderVec2 m_vRenderSize;
    spShaderVec2 m_vInverseRenderSize;

    spShaderVec2 m_vViewportSize;
    spShaderVec2 m_vInverseViewportSize;

    ezUInt32 m_uiOptions;
    spShaderBool m_bHDREnabled;
    float m_fHDRMaxNits;
    float m_fHDRWhitePoint;
  };

  class SP_RPI_DLL spRenderContext
  {
    friend class spSceneContext;

  public:
    ~spRenderContext();

#pragma region Frame Management

    /// \brief Sets the current context's command list with an existing instance.
    /// \param pCommandList The new command list.
    void SetCommandList(ezSharedPtr<RHI::spCommandList> pCommandList);

    /// \brief Creates a new  context's command list from the given descriptor.
    /// \param description The command list descriptor.
    void SetCommandList(const RHI::spCommandListDescription& description);

    /// \brief Returns the current context's command list.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<RHI::spCommandList> GetCommandList() const { return m_pCommandList; }

    /// \brief Gets the device used by this context.
    EZ_NODISCARD EZ_ALWAYS_INLINE const spSceneContext* GetSceneContext() const { return m_pSceneContext; }

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

#pragma endregion

#pragma region Shared Data

    EZ_NODISCARD EZ_ALWAYS_INLINE const spConstantBuffer<spRenderFrameData, RHI::spBufferUsage::DoubleBuffered>& GetFrameDataBuffer() const { return m_FrameDataBuffer; }

    EZ_NODISCARD const spRenderContextData& GetExtractionData() const;

    spRenderContextData& GetExtractionData();

#pragma endregion

#pragma region Shader Management

    /// \brief Gets the \a spShaderManager instance associated with the \a spRenderContext.
    EZ_NODISCARD EZ_ALWAYS_INLINE spShaderManager* GetShaderManager() const { return m_pShaderManager.Borrow(); }

#pragma endregion

#pragma region Render Context State

    /// \brief Resets the current state of the \a spRenderContext.
    void Reset();

#pragma endregion

  private:
    explicit spRenderContext(const spSceneContext* pSceneContext);

    const spSceneContext* m_pSceneContext{nullptr};

    ezSharedPtr<RHI::spCommandList> m_pCommandList{nullptr};
    ezMutex m_CommandListLock;

    spConstantBuffer<spRenderFrameData, RHI::spBufferUsage::DoubleBuffered> m_FrameDataBuffer;

    spRenderContextData m_Data[2];

    ezUniquePtr<spShaderManager> m_pShaderManager{nullptr};
  };
} // namespace RPI
