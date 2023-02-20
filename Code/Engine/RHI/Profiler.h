#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

class spDevice;

/// \brief A profiler suited to profile an entire frame.
class SP_RHI_DLL spFrameProfiler : public spDeviceResource
{
  EZ_ADD_DYNAMIC_REFLECTION(spFrameProfiler, spDeviceResource);

  friend class spDevice;

public:
  /// \brief Starts profiling the frame.
  virtual void Begin() = 0;

  /// \brief Stops profiling the frame.
  virtual void End() = 0;
};

/// \brief A profiler used for scope.
class SP_RHI_DLL spScopeProfiler : public spDeviceResource
{
  EZ_ADD_DYNAMIC_REFLECTION(spScopeProfiler, spDeviceResource);

public:
  /// \brief Gets the name of the profiled scope.
  EZ_NODISCARD EZ_ALWAYS_INLINE ezStringView GetScopeName() const { return m_sName; }

  /// \brief Gets the time at which the profiler was started.
  virtual ezTime GetBeginTime() = 0;

  /// \brief Gets the time at which the profiler was stopped.
  virtual ezTime GetEndTime() = 0;

protected:
  ezStringView m_sName;
};
