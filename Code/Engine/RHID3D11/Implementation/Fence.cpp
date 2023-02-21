#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/Fence.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFenceD3D11, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void spFenceD3D11::ReleaseResource()
{
  if (IsReleased())
    return;

  if (m_hEvent != nullptr)
  {
    CloseHandle(m_hEvent);
    m_hEvent = nullptr;
  }

  SP_RHI_DX11_RELEASE(m_pD3D11Fence);

  m_bReleased = true;
}

bool spFenceD3D11::IsReleased() const
{
  return m_pD3D11Fence == nullptr;
}

bool spFenceD3D11::IsSignaled()
{
  return Wait(ezTime::Zero());
}

void spFenceD3D11::Reset()
{
  m_uiCurrentFenceValue = 0;

  if (m_hEvent != nullptr)
  {
    SetEvent(m_hEvent);
    ResetEvent(m_hEvent);
  }
}

void spFenceD3D11::Raise(ID3D11DeviceContext4* pDeviceContext)
{
  Raise(m_uiCurrentFenceValue + 1, pDeviceContext);
}

void spFenceD3D11::Raise(ezUInt64 uiValue, ID3D11DeviceContext4* pDeviceContext)
{
  EZ_ASSERT_DEV(uiValue > m_uiCurrentFenceValue, "Invalid fence value.");
  m_uiCurrentFenceValue = uiValue;

  const HRESULT res = pDeviceContext->Signal(m_pD3D11Fence, uiValue);
  EZ_HRESULT_TO_ASSERT(res);
}

bool spFenceD3D11::Wait(ezTime timeout) const
{
  return Wait(m_uiCurrentFenceValue, timeout);
}

bool spFenceD3D11::Wait(ezUInt64 uiValue, ezTime timeout) const
{
  if (m_pD3D11Fence->GetCompletedValue() < uiValue)
  {
    EZ_HRESULT_TO_ASSERT(m_pD3D11Fence->SetEventOnCompletion(uiValue, m_hEvent));
    const auto t = static_cast<DWORD>(timeout.GetMilliseconds());
    return WaitForSingleObjectEx(m_hEvent, t, FALSE) == WAIT_OBJECT_0;
  }

  return true;
}

spFenceD3D11::spFenceD3D11(spDeviceD3D11* pDevice, const spFenceDescription& description)
  : spFence(description)
{
  m_pDevice = pDevice;

  pDevice->GetD3D11Device()->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_ID3D11Fence, reinterpret_cast<void**>(&m_pD3D11Fence));
  m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  EZ_ASSERT_DEV(m_hEvent != nullptr, "Failed to D3D11 fence create event.");

  if (description.m_bSignaled)
    SetEvent(m_hEvent);

  m_bReleased = false;
}

spFenceD3D11::~spFenceD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Fence);
