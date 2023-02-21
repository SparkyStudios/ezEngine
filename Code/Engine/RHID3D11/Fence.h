#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <Foundation/Threading/ThreadSignal.h>

#include <RHI/Fence.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spFenceD3D11 final : public spFence
{
  friend class spDeviceResourceFactoryD3D11;

  EZ_ADD_DYNAMIC_REFLECTION(spFenceD3D11, spFence);

public:
  // spDeviceResource

  void ReleaseResource() override;
  bool IsReleased() const override;

  // spFence

  bool IsSignaled() override;

  // spFenceD3D11

  void Reset();

  void Raise();

  bool Wait();
  bool Wait(ezTime timeout);

  spFenceD3D11(spDeviceD3D11* pDevice, const spFenceDescription& description);
  ~spFenceD3D11() override;

private:
  ezThreadSignal m_ThreadSignal;

  bool m_bSignaled{false};
};
