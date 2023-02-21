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

  m_ThreadSignal.RaiseSignal();
  m_ThreadSignal.ClearSignal();

  m_bReleased = true;
}

bool spFenceD3D11::IsReleased() const
{
  return m_bReleased;
}

bool spFenceD3D11::IsSignaled()
{
  return Wait(ezTime::Zero());
}

void spFenceD3D11::Reset()
{
  if (!m_bSignaled)
    return;

  m_bSignaled = false;
  m_ThreadSignal.ClearSignal();
}

void spFenceD3D11::Raise()
{
  if (m_bSignaled)
    return;

  m_bSignaled = true;
  m_ThreadSignal.RaiseSignal();
}

bool spFenceD3D11::Wait()
{
  if (!m_bSignaled)
    m_ThreadSignal.WaitForSignal();

  m_bSignaled = true;

  return m_bSignaled;
}

bool spFenceD3D11::Wait(ezTime timeout)
{
  if (!m_bSignaled)
    m_bSignaled = m_ThreadSignal.WaitForSignal(timeout) == ezThreadSignal::WaitResult::Signaled;

  return m_bSignaled;
}

spFenceD3D11::spFenceD3D11(spDeviceD3D11* pDevice, const spFenceDescription& description)
  : spFence(description)
  , m_ThreadSignal(ezThreadSignal::Mode::ManualReset)
{
  m_pDevice = pDevice;

  if (description.m_bSignaled)
    m_ThreadSignal.RaiseSignal();
  else
    m_ThreadSignal.ClearSignal();

  m_bSignaled = description.m_bSignaled;
}

spFenceD3D11::~spFenceD3D11()
{
  m_pDevice->GetResourceManager()->ReleaseResource(this);
}

EZ_STATICLINK_FILE(RHID3D11, RHID3D11_Implementation_Fence);
