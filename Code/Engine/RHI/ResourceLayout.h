#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a single element in a resource layout.
  struct spResourceLayoutElementDescription : ezHashableStruct<spResourceLayoutElementDescription>
  {
    /// \brief The layout element name.
    ezHashedString m_sName;

    /// \brief The layout element type.
    ezEnum<spShaderResourceType> m_eType;

    /// \brief A flag specifying options enabled for the layout element.
    ezBitflags<spResourceLayoutElementOptions> m_eOptions;
  };

  /// \brief Describes the layout of \a spShaderResource objects for a \a spRenderingPipeline.
  struct spResourceLayoutDescription : ezHashableStruct<spResourceLayoutDescription>
  {
    /// \brief The list of shader stages where the layout is used.
    ezBitflags<spShaderStage> m_eShaderStage;

    /// \brief An array of \a spResourceLayoutElementDescription objects, describing the properties of each resource
    /// element in the \a spResourceLayout.
    ezDynamicArray<spResourceLayoutElementDescription> m_Elements;
  };

  class SP_RHI_DLL spResourceLayout : public spDeviceResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spResourceLayout, spDeviceResource);

    friend class spDeviceResourceFactory;

  public:
    [[nodiscard]] EZ_ALWAYS_INLINE ezBitflags<spShaderStage> GetShaderStages() const { return m_Description.m_eShaderStage; }

    [[nodiscard]] EZ_ALWAYS_INLINE const ezDynamicArray<spResourceLayoutElementDescription>& GetElements() const { return m_Description.m_Elements; }

    /// \brief Gets the number of elements in the layout.
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetElementCount() const { return m_Description.m_Elements.GetCount(); }

    [[nodiscard]] ezUInt32 GetElementIndex(const ezTempHashedString& sName) const;

    [[nodiscard]] EZ_ALWAYS_INLINE const spResourceLayoutElementDescription& GetElement(ezUInt32 uiIndex) const
    {
      EZ_ASSERT_DEV(uiIndex < m_Description.m_Elements.GetCount(), "Index out of bounds.");
      return m_Description.m_Elements[uiIndex];
    }

    /// \brief Gets the description used to create the layout.
    [[nodiscard]] EZ_ALWAYS_INLINE const spResourceLayoutDescription& GetDescription() const { return m_Description; }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    [[nodiscard]] EZ_ALWAYS_INLINE ezUInt32 GetDynamicBufferCount() const { return m_uiDynamicBufferCount; }
#endif

  protected:
    explicit spResourceLayout(spResourceLayoutDescription description);

    spResourceLayoutDescription m_Description;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezUInt32 m_uiDynamicBufferCount{0};
#endif
  };
} // namespace RHI
