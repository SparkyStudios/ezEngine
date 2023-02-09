#include <RHI/RHIPCH.h>

#include <RHI/ResourceLayout.h>

spResourceLayout::spResourceLayout(spResourceLayoutDescription description)
  : m_Description(std::move(description))
{
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  for (const auto& element : m_Description.m_Elements)
    if (element.m_eOptions.IsSet(spResourceLayoutElementOptions::DynamicBinding))
      m_uiDynamicBufferCount++;
#endif
}

EZ_STATICLINK_FILE(RHI, RHI_Implementation_ResourceLayout);
