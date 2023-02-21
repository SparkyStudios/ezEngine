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
  void Reset() override;

  // spFenceD3D11

  void Raise(ID3D11DeviceContext4* pDeviceContext);
  void Raise(ezUInt64 uiValue, ID3D11DeviceContext4* pDeviceContext);

  bool Wait(ezTime timeout) const;
  bool Wait(ezUInt64 uiValue, ezTime timeout) const;

  spFenceD3D11(spDeviceD3D11* pDevice, const spFenceDescription& description);
  ~spFenceD3D11() override;

private:
  ID3D11Fence* m_pD3D11Fence{nullptr};
  HANDLE m_hEvent{nullptr};

  ezUInt64 m_uiCurrentFenceValue{0};
};
