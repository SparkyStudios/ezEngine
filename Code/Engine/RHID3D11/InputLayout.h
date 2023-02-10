#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Input.h>

class spDeviceD3D11;

class SP_RHID3D11_DLL spInputLayoutD3D11 final : public spInputLayout
{
  // spDeviceResource

public:
  void ReleaseResource() override;
  EZ_NODISCARD bool IsReleased() const override;

  // spInputLayoutD3D11

public:
  spInputLayoutD3D11(spDeviceD3D11* pDevice, const spInputLayoutDescription& desc);
  ~spInputLayoutD3D11();
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHID3D11_DLL, spInputLayoutD3D11);
