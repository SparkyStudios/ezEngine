#include <RHI/RHIPCH.h>

#include <RHI/Fence.h>

#pragma region spFence

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spFence, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spFence::spFence(spFenceDescription description)
  : m_Description(std::move(description))
{
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Fence);
