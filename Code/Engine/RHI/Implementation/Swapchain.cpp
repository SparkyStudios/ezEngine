#include <RHI/RHIPCH.h>

#include <RHI/Swapchain.h>

namespace RHI
{
#pragma region spRenderingSurface

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderingSurface, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

#pragma endregion

#pragma region spSwapchain

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSwapchain, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  spSwapchain::spSwapchain(spSwapchainDescription description)
    : m_Description(std::move(description))
  {
  }

#pragma endregion
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Swapchain);
