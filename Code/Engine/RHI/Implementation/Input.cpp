#include <RHI/RHIPCH.h>

#include <RHI/Input.h>

namespace RHI
{
#pragma region spInputLayout

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spInputLayout, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spInputLayout::spInputLayout(spInputLayoutDescription description)
    : m_Description(std::move(description))
  {
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Input);
