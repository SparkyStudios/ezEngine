#include <RHIMTL/RHIMTLPCH.h>

#include <RHIMTL/Device.h>
#include <RHIMTL/InputLayout.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spInputLayoutMTL, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  void spInputLayoutMTL::ReleaseResource()
  {
    if (IsReleased())
      return;

    m_bReleased = true;
  }

  bool spInputLayoutMTL::IsReleased() const
  {
    return m_bReleased;
  }

  spInputLayoutMTL::spInputLayoutMTL(spDeviceMTL* pDevice, const spInputLayoutDescription& description)
    : spInputLayout(description)
  {
    m_pDevice = pDevice;
  }

  spInputLayoutMTL::~spInputLayoutMTL()
  {
    m_pDevice->GetResourceManager()->ReleaseResource(this);
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHIMTL, RHIMTL_Implementation_InputLayout);
