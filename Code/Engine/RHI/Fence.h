#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a Fence, for creation using a \a spDeviceResourceFactory.
  struct spFenceDescription : ezHashableStruct<spFenceDescription>
  {
    /// \brief Constructs a spFenceDescription.
    spFenceDescription() = default;

    /// \brief Constructs a spFenceDescription.
    /// \param bSignaled The signaled state of the Fence.
    spFenceDescription(bool bSignaled)
      : ezHashableStruct()
      , m_bSignaled(bSignaled)
    {
    }

    /// \brief Compares this \a spFenceDescription with the \a other description for equality.
    EZ_ALWAYS_INLINE bool operator==(const spFenceDescription& other) const
    {
      return m_bSignaled == other.m_bSignaled;
    }

    /// \brief Compares this \a spFenceDescription with the \a other description for inequality
    EZ_ALWAYS_INLINE bool operator!=(const spFenceDescription& other) const
    {
      return !(*this == other);
    }

    /// \brief The signaled state of the Fence.
    /// \note This value is not used on OpenGL backed.
    bool m_bSignaled{false};
  };

  /// \brief A Fence. Can be used to wait for tasks to finish and synchronize
  /// the CPU and the GPU.
  class SP_RHI_DLL spFence : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spFence, spDeviceResource);

  public:
    /// \brief Gets a value indicating whether the fence is currently signaled.
    EZ_NODISCARD virtual bool IsSignaled() = 0;

  protected:
    explicit spFence(spFenceDescription description);

    spFenceDescription m_Description;
  };
} // namespace RHI
