#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Profiler.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>

#include <Foundation/Threading/ThreadUtils.h>

namespace RHI
{
#pragma region spFrameProfilerD3D11

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFrameProfilerD3D11, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spFrameProfilerD3D11::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_DX11_RELEASE(m_pDisjointQuery);

    m_bReleased = true;
  }

  bool spFrameProfilerD3D11::IsReleased() const
  {
    return m_pDisjointQuery == nullptr;
  }

  void spFrameProfilerD3D11::Begin()
  {
    ID3D11DeviceContext* pContext = static_cast<spDeviceD3D11*>(m_pDevice)->GetD3D11DeviceContext();
    pContext->Begin(m_pDisjointQuery);

    m_fInvTicksPerSecond = -1.0;
  }

  void spFrameProfilerD3D11::End()
  {
    ID3D11DeviceContext* pContext = static_cast<spDeviceD3D11*>(m_pDevice)->GetD3D11DeviceContext();
    pContext->End(m_pDisjointQuery);

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
    while (pContext->GetData(m_pDisjointQuery, &data, sizeof(data), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
      ezThreadUtils::YieldTimeSlice();

    m_fInvTicksPerSecond = 1.0 / static_cast<double>(data.Frequency);

    if (m_bSyncTimeNeeded)
    {
      D3D11_QUERY_DESC desc;
      desc.Query = D3D11_QUERY_TIMESTAMP;
      desc.MiscFlags = 0;

      spScopedD3D11Resource<ID3D11Query> pQuery;
      const HRESULT res = static_cast<spDeviceD3D11*>(m_pDevice)->GetD3D11Device()->CreateQuery(&desc, &pQuery);
      EZ_HRESULT_TO_ASSERT(res);

      pContext->End(*pQuery);

      ezUInt64 uiTimestamp;
      while (pContext->GetData(*pQuery, &uiTimestamp, sizeof(uiTimestamp), 0) != S_OK)
        ezThreadUtils::YieldTimeSlice();

      m_SyncTimeDiff = ezTime::Now() - ezTime::Seconds(static_cast<double>(uiTimestamp) * m_fInvTicksPerSecond);
      m_bSyncTimeNeeded = false;
    }
  }

  spFrameProfilerD3D11::spFrameProfilerD3D11(spDeviceD3D11* pDevice)
  {
    m_pDevice = pDevice;

    D3D11_QUERY_DESC queryDesc;
    queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
    queryDesc.MiscFlags = 0;

    const HRESULT res = pDevice->GetD3D11Device()->CreateQuery(&queryDesc, &m_pDisjointQuery);
    EZ_HRESULT_TO_ASSERT(res);

    pDevice->GetResourceManager()->RegisterResource(this);

    m_bReleased = false;
  }

  spFrameProfilerD3D11::~spFrameProfilerD3D11()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

#pragma endregion

#pragma region spScopeProfilerD3D11

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spScopeProfilerD3D11, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spScopeProfilerD3D11::ReleaseResource()
  {
    if (IsReleased())
      return;

    SP_RHI_DX11_RELEASE(m_pBeginQuery);
    SP_RHI_DX11_RELEASE(m_pEndQuery);

    m_bReleased = true;
  }

  bool spScopeProfilerD3D11::IsReleased() const
  {
    return m_pBeginQuery == nullptr && m_pEndQuery == nullptr;
  }

  ezTime spScopeProfilerD3D11::GetBeginTime()
  {
    return GetTime(m_pBeginQuery);
  }

  ezTime spScopeProfilerD3D11::GetEndTime()
  {
    return GetTime(m_pEndQuery);
  }

  spScopeProfilerD3D11::spScopeProfilerD3D11(spDeviceD3D11* pDevice)
  {
    m_pDevice = pDevice;

    D3D11_QUERY_DESC beginQueryDesc;
    beginQueryDesc.Query = D3D11_QUERY_TIMESTAMP;
    beginQueryDesc.MiscFlags = 0;

    D3D11_QUERY_DESC endQueryDesc;
    endQueryDesc.Query = D3D11_QUERY_TIMESTAMP;
    endQueryDesc.MiscFlags = 0;

    HRESULT res = pDevice->GetD3D11Device()->CreateQuery(&beginQueryDesc, &m_pBeginQuery);
    EZ_HRESULT_TO_ASSERT(res);

    res = pDevice->GetD3D11Device()->CreateQuery(&endQueryDesc, &m_pEndQuery);
    EZ_HRESULT_TO_ASSERT(res);

    pDevice->GetResourceManager()->RegisterResource(this);

    m_bReleased = false;
  }

  spScopeProfilerD3D11::~spScopeProfilerD3D11()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }

  void spScopeProfilerD3D11::Begin(ezStringView sName, ID3D11DeviceContext* pContext)
  {
    m_sName = sName;
    pContext->End(m_pBeginQuery);
  }

  void spScopeProfilerD3D11::End(ID3D11DeviceContext* pContext)
  {
    pContext->End(m_pEndQuery);
  }

  ezTime spScopeProfilerD3D11::GetTime(ID3D11Query* pQuery)
  {
    ID3D11DeviceContext* pContext = static_cast<spDeviceD3D11*>(m_pDevice)->GetD3D11DeviceContext();
    const ezSharedPtr<spFrameProfilerD3D11> pProfiler = m_pDevice->GetFrameProfiler().Downcast<spFrameProfilerD3D11>();

    ezUInt64 uiTimestamp;
    if (FAILED(pContext->GetData(pQuery, &uiTimestamp, sizeof(uiTimestamp), D3D11_ASYNC_GETDATA_DONOTFLUSH)))
      return ezTime::Zero();

    if (pProfiler->m_fInvTicksPerSecond == 0.0)
      return ezTime::Zero();

    return ezTime::Seconds(static_cast<double>(uiTimestamp) * pProfiler->m_fInvTicksPerSecond) + pProfiler->m_SyncTimeDiff;
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Profiler);
