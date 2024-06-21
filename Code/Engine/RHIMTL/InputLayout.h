#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/Input.h>

namespace RHI
{
  class spDeviceMTL;

  class SP_RHIMTL_DLL spInputLayoutMTL final : public spInputLayout
  {
    EZ_ADD_DYNAMIC_REFLECTION(spInputLayoutMTL, spInputLayout);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    [[nodiscard]] bool IsReleased() const override;

    // spInputLayoutMTL

  public:
    spInputLayoutMTL(spDeviceMTL* pDevice, const spInputLayoutDescription& description);
    ~spInputLayoutMTL() override;
  };
} // namespace RHI
