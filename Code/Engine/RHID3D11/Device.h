#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <d3d11.h>

#include <RHI/Device.h>

class SP_RHID3D11_DLL spDeviceD3D11 final : public spDevice
{
public:
  EZ_NODISCARD EZ_ALWAYS_INLINE ID3D11Device* GetD3D11Device() const { return m_pD3D11Device; }

private:
  ID3D11Device* m_pD3D11Device;
};
