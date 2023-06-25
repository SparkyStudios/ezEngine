#include <RHI/RHIPCH.h>

#include <RHI/Sampler.h>

namespace RHI
{
#pragma region spSamplerState

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSamplerState, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

#pragma endregion

#pragma region spSampler

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSampler, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Sampler);
