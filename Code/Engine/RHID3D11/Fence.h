#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <Foundation/Threading/ThreadSignal.h>

#include <RHI/Fence.h>

class spFenceD3D11 final : public spFence
{
  friend class spDeviceResourceFactoryD3D11;

public:
  // spDeviceResource

  void ReleaseResource() override;
  bool IsReleased() const override;

  // spFence

  bool IsSignaled() override;
  void Reset() override;

  // spFenceD3D11

  void Raise();
  bool Wait(ezTime timeout);

private:
  spFenceD3D11(spDeviceD3D11* pDevice, const spFenceDescription& description);
  ~spFenceD3D11() override = default;

  ezThreadSignal m_ThreadSignal;

  bool m_bReleased{false};
  bool m_bSignaled{false};
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spFenceD3D11);
