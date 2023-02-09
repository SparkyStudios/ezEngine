#include <RHI/RHIPCH.h>

#include <RHI/ResourceSet.h>

spResourceSet::spResourceSet(spResourceSetDescription description)
  : m_Description(std::move(description))
{
}

EZ_STATICLINK_FILE(RHI, RHI_Implementation_ResourceSet);
