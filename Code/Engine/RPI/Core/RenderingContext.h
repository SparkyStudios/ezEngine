#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/CommandList.h>
#include <RHI/Device.h>

struct alignas(16) spRenderingContextData
{
};

class SP_RPI_DLL spRenderingContext
{
public:
  explicit spRenderingContext(spDevice* pDevice);
  ~spRenderingContext();

  /// \brief Sets the current context's command list with an existing instance.
  /// \param pCommandList The new command list.
  void SetCommandList(ezSharedPtr<spCommandList> pCommandList);

  /// \brief Creates a new  context's command list from the given descriptor.
  /// \param description The command list descriptor.
  EZ_ALWAYS_INLINE void SetCommandList(const spCommandListDescription& description) { m_pCommandList = m_pDevice->GetResourceFactory()->CreateCommandList(description); }

  /// \brief Returns the current context's command list.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezSharedPtr<spCommandList> GetCommandList() const { return m_pCommandList; }

  /// \brief Gets the device used by this context.
  EZ_NODISCARD EZ_ALWAYS_INLINE spDevice* GetDevice() const { return m_pDevice; }

  /// \brief Marks the beginning of a render process.
  /// Render passes will be executed after this call and commands will be collected
  /// in the current command list.
  /// When the render process is finished, the command list will be executed by
  /// calling \a EndFrame.
  void BeginFrame();

  /// \brief Marks the end of a render process.
  /// This will execute all commands that were collected in the current context
  /// and reset the command list for the next render process.
  void EndFrame(ezSharedPtr<spFence> pFence = nullptr);

  /// \brief Resets the current state of the \a spRenderingContext.
  void Reset();

private:
  spDevice* m_pDevice;
  ezSharedPtr<spCommandList> m_pCommandList;
};
