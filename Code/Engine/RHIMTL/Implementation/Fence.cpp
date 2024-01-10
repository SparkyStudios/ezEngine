#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Device.h>
#include <RHIMTL/Fence.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFenceMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spFenceMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_ThreadSignal.RaiseSignal();
    m_ThreadSignal.ClearSignal();

    m_bReleased = true;
  }

  bool spFenceMTL::IsReleased() const
  {
    return m_bReleased;
  }

  bool spFenceMTL::IsSignaled()
  {
    return Wait(ezTime::MakeZero());
  }

  void spFenceMTL::Reset()
  {
    if (!m_bSignaled)
      return;

    m_bSignaled = false;
    m_ThreadSignal.ClearSignal();
  }

  void spFenceMTL::Raise()
  {
    if (m_bSignaled)
      return;

    m_bSignaled = true;
    m_ThreadSignal.RaiseSignal();
  }

  bool spFenceMTL::Wait()
  {
    if (!m_bSignaled)
      m_ThreadSignal.WaitForSignal();

    m_bSignaled = true;

    return m_bSignaled;
  }

  bool spFenceMTL::Wait(ezTime timeout)
  {
    if (!m_bSignaled)
      m_bSignaled = m_ThreadSignal.WaitForSignal(timeout) == ezThreadSignal::WaitResult::Signaled;

    return m_bSignaled;
  }

  spFenceMTL::spFenceMTL(spDeviceMTL* pDevice, const spFenceDescription& description)
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

  spFenceMTL::~spFenceMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_Fence);
