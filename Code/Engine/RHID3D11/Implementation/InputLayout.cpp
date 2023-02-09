#include <RHID3D11/RHID3D11PCH.h>

#include <RHID3D11/Device.h>
#include <RHID3D11/InputLayout.h>

void spInputLayoutD3D11::ReleaseResource()
{
  m_bReleased = true;
}

bool spInputLayoutD3D11::IsReleased() const
{
  return m_bReleased;
}

spInputLayoutD3D11::spInputLayoutD3D11(spDeviceD3D11* pDevice, const spInputLayoutDescription& description)
  : spInputLayout(description)
{
  m_pDevice = pDevice;
}
