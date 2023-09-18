#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/CommandList.h>
#include <RHI/Device.h>
#include <RHI/Fence.h>

#include <RPI/Core/RenderContext.h>

namespace RPI
{
  class spRenderPipeline;

  class SP_RPI_DLL spSceneContext
  {
  public:
    explicit spSceneContext(RHI::spDevice* pDevice);

    /// \brief Gets the \a spRenderContext which stores drawing data for this scene.
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezUniquePtr<spRenderContext>& GetRenderContext() const { return m_pRenderContext; }

    /// \brief Adds a \a spRenderingPipeline to execute when drawing this \a spSceneContext.
    void AddPipeline(spRenderPipeline* pPipeline);

    /// \brief Begins a draw operation.
    void BeginFrame();

    /// \brief Draws the current \a spSceneContext on its \a spRenderTarget.
    void Draw();

    /// \brief Ends a draw operation.
    void EndFrame();

    /// \brief Blocks the calling thread until this \a spSceneContext
    /// is idle (all outstanding draw operations are completed).
    void WaitForIdle();

  private:
    RHI::spDevice* m_pDevice{nullptr};

    ezSharedPtr<RHI::spCommandList> m_pCommandList{nullptr};
    ezSharedPtr<RHI::spFence> m_pFence{nullptr};

    ezUniquePtr<spRenderContext> m_pRenderContext{nullptr};

    ezList<spRenderPipeline*> m_RenderPipelines;
  };
} // namespace RPI