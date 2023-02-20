#include <RHI/RHIPCH.h>

#include <RHI/ResourceSet.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spResourceSet, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spResourceSet::spResourceSet(spResourceSetDescription description)
  : m_Description(std::move(description))
{
}

EZ_STATICLINK_FILE(RHI, RHI_Implementation_ResourceSet);
