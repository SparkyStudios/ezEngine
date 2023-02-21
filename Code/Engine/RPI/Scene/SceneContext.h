#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Pipeline/RenderPipeline.h>

class spRenderingContext;

class SP_RPI_DLL spSceneContext
{
public:
  explicit spSceneContext(spDevice* pDevice);

  /// \brief Gets the \a spRenderingContext which stores drawing data for this scene.
  EZ_NODISCARD EZ_ALWAYS_INLINE spRenderingContext* GetRenderingContext() const { return m_pRenderingContext.Borrow(); }

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
  spDevice* m_pDevice{nullptr};

  ezSharedPtr<spCommandList> m_pCommandList{nullptr};
  ezUniquePtr<spRenderingContext> m_pRenderingContext{nullptr};

  ezList<spRenderPipeline*> m_RenderPipelines;

  ezSharedPtr<spFence> m_pFence{nullptr};
};
