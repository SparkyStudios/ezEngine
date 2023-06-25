#include <RHI/RHIPCH.h>

#include <RHI/Profiler.h>

namespace RHI
{
#pragma region spFrameProfiler

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFrameProfiler, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

#pragma endregion

#pragma region spScopeProfiler

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spScopeProfiler, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Profiler);
