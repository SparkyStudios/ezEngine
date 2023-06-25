#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Profiler.h>

namespace RHI
{
  class spDeviceD3D11;

  /// \brief A profiler suited to profile an entire frame.
  class SP_RHID3D11_DLL spFrameProfilerD3D11 final : public spFrameProfiler
  {
    friend class spScopeProfilerD3D11;

    EZ_ADD_DYNAMIC_REFLECTION(spFrameProfilerD3D11, spFrameProfiler);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spFrameProfiler

  public:
    void Begin() override;
    void End() override;

    // spFrameProfilerD3D11

  public:
    spFrameProfilerD3D11(spDeviceD3D11* pDevice);
    ~spFrameProfilerD3D11() override;

  private:
    ID3D11Query* m_pDisjointQuery{nullptr};

    ezTime m_SyncTimeDiff;
    bool m_bSyncTimeNeeded{true};

    double m_fInvTicksPerSecond{-1.0};
  };

  /// \brief A profiler used for scope.
  class SP_RHID3D11_DLL spScopeProfilerD3D11 final : public spScopeProfiler
  {
    EZ_ADD_DYNAMIC_REFLECTION(spScopeProfilerD3D11, spScopeProfiler);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spScopeProfiler

  public:
    ezTime GetBeginTime() override;
    ezTime GetEndTime() override;

    // spScopeProfilerD3D11

  public:
    spScopeProfilerD3D11(spDeviceD3D11* pDevice);
    ~spScopeProfilerD3D11() override;

    /// \brief Starts the profiler.
    /// \param sName The name of the scope.
    /// \param pContext The D3D11 device context to use. Mostly coming from a command list.
    void Begin(ezStringView sName, ID3D11DeviceContext* pContext);

    /// \brief Stops the profiler.
    /// \param pContext The D3D11 device context to use. Mostly coming from a command list.
    void End(ID3D11DeviceContext* pContext);

  private:
    ezTime GetTime(ID3D11Query* pQuery);

    ID3D11Query* m_pBeginQuery{nullptr};
    ID3D11Query* m_pEndQuery{nullptr};
  };
} // namespace RHI
