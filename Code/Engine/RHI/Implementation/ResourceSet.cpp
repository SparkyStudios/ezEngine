#include <RHI/RHIPCH.h>

#include <RHI/ResourceSet.h>

spResourceSet::spResourceSet(spResourceSetDescription description)
  : m_Description(std::move(description))
{
}
