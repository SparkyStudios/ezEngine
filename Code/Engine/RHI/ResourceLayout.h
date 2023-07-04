#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a single element in a resource layout.
  struct spResourceLayoutElementDescription : ezHashableStruct<spResourceLayoutElementDescription>
  {
    ezHashedString m_sName;
    ezEnum<spShaderResourceType> m_eType;
    ezBitflags<spShaderStage> m_eShaderStage;
    ezBitflags<spResourceLayoutElementOptions> m_eOptions;
  };

  /// \brief Describes the layout of \a spShaderResource objects for a \a spRenderingPipeline.
  struct spResourceLayoutDescription : ezHashableStruct<spResourceLayoutDescription>
  {
    /// \brief An array of \a spResourceLayoutElementDescription objects, describing the properties of each resource
    /// element in the \a spResourceLayout.
    ezDynamicArray<spResourceLayoutElementDescription> m_Elements;
  };

  class SP_RHI_DLL spResourceLayout : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spResourceLayout, spDeviceResource);

    friend class spDeviceResourceFactory;

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<spResourceLayoutElementDescription>& GetElements() const { return m_Description.m_Elements; }

    /// \brief Gets the number of elements in the layout.
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetElementCount() const { return m_Description.m_Elements.GetCount(); }

    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetElementIndex(const ezHashedString& sName) const
    {
      for (ezUInt32 i = 0, l = m_Description.m_Elements.GetCount(); i < l; ++i)
        if (m_Description.m_Elements[i].m_sName == sName)
          return i;

      return ezInvalidIndex;
    }

    /// \brief Gets the description used to create the layout.
    EZ_NODISCARD EZ_ALWAYS_INLINE const spResourceLayoutDescription& GetDescription() const { return m_Description; }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    EZ_NODISCARD EZ_ALWAYS_INLINE ezUInt32 GetDynamicBufferCount() const
    {
      return m_uiDynamicBufferCount;
    }
#endif

  protected:
    explicit spResourceLayout(spResourceLayoutDescription description);

    spResourceLayoutDescription m_Description;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezUInt32 m_uiDynamicBufferCount{0};
#endif
  };
} // namespace RHI
