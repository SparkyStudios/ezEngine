#pragma once

#include <RHID3D11/RHID3D11DLL.h>

#include <RHI/Input.h>

namespace RHI
{
  class spDeviceD3D11;

  class SP_RHID3D11_DLL spInputLayoutD3D11 final : public spInputLayout
  {
    EZ_ADD_DYNAMIC_REFLECTION(spInputLayoutD3D11, spInputLayout);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    [[nodiscard]] bool IsReleased() const override;

    // spInputLayoutD3D11

  public:
    spInputLayoutD3D11(spDeviceD3D11* pDevice, const spInputLayoutDescription& description);
    ~spInputLayoutD3D11() override;
  };
} // namespace RHI
