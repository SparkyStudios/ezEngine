#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

class spDevice;

/// \brief A profiler suited to profile an entire frame.
class SP_RHI_DLL spFrameProfiler : public spDeviceResource
{
  friend class spDevice;

public:
  /// \brief Starts profiling the frame.
  virtual void Begin() = 0;

  /// \brief Stops profiling the frame.
  virtual void End() = 0;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spFrameProfiler);

/// \brief A profiler used for scope.
class SP_RHI_DLL spScopeProfiler : public spDeviceResource
{
public:
  /// \brief Gets the name of the profiled scope.
  EZ_NODISCARD EZ_ALWAYS_INLINE const ezString& GetScopeName() const { return m_sName; }

  /// \brief Gets the time at which the profiler was started.
  virtual ezTime GetBeginTime() = 0;

  /// \brief Gets the time at which the profiler was stopped.
  virtual ezTime GetEndTime() = 0;

protected:
  ezString m_sName;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spScopeProfiler);
