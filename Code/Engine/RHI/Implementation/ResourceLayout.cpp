#include <RHI/RHIPCH.h>

#include <RHI/ResourceLayout.h>

namespace RHI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spResourceLayout, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  ezUInt32 spResourceLayout::GetElementIndex(const ezHashedString& sName) const
  {
    for (ezUInt32 i = 0, l = m_Description.m_Elements.GetCount(); i < l; ++i)
      if (m_Description.m_Elements[i].m_sName == sName)
        return i;

    return ezInvalidIndex;
  }

  spResourceLayout::spResourceLayout(spResourceLayoutDescription description)
    : m_Description(std::move(description))
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    for (const auto& element : m_Description.m_Elements)
      if (element.m_eOptions.IsSet(spResourceLayoutElementOptions::DynamicBinding))
        m_uiDynamicBufferCount++;
#endif
  }
} // namespace RHI

EZ_STATICLINK_FILE(RHI, RHI_Implementation_ResourceLayout);
