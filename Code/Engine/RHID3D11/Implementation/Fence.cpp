#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Fence.h>

void spFenceD3D11::ReleaseResource()
{
  m_ThreadSignal.RaiseSignal();
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
    return ;

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

bool spFenceD3D11::Wait(ezTime timeout)
{
  if (!m_bSignaled)
    m_bSignaled = m_ThreadSignal.WaitForSignal(timeout) == ezThreadSignal::WaitResult::Signaled;

  return m_bSignaled;
}

spFenceD3D11::spFenceD3D11(spDeviceD3D11* pDevice, const spFenceDescription& description)
  : spFence()
  , m_ThreadSignal(ezThreadSignal::Mode::ManualReset)
{
  m_pDevice = pDevice;

  if (description.m_bSignaled)
    m_ThreadSignal.RaiseSignal();
  else
    m_ThreadSignal.ClearSignal();

  m_bSignaled = description.m_bSignaled;
}
