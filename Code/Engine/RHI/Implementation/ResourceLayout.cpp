#include <RHI/RHIPCH.h>

#include <RHI/ResourceLayout.h>

spResourceLayout::spResourceLayout(spResourceLayoutDescription description)
  : m_Description(std::move(description))
{
}
