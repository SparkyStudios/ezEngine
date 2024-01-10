#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <Foundation/Threading/ThreadSignal.h>

#include <RHI/Fence.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spFenceMTL final : public spFence
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spFenceMTL, spFence);

  public:
    // spDeviceResource

    void ReleaseResource() override;
    bool IsReleased() const override;

    // spFence

    bool IsSignaled() override;

    // spFenceMTL

    void Reset();

    void Raise();

    bool Wait();
    bool Wait(ezTime timeout);

    spFenceMTL(spDeviceMTL* pDevice, const spFenceDescription& description);
    ~spFenceMTL() override;

  private:
    ezThreadSignal m_ThreadSignal;

    bool m_bSignaled{false};
  };
} // namespace RHI
