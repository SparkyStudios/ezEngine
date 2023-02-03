#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

/// \brief Describes a single element in a resource layout.
struct spResourceLayoutElementDescription : public ezHashableStruct<spResourceLayoutElementDescription>
{
  ezHashedString m_sName;
  ezEnum<spShaderResourceType> m_eType;
  ezBitflags<spShaderStage> m_eShaderStage;
  ezBitflags<spResourceLayoutElementOptions> m_eOptions;
};

/// \brief Describes the layout of \see spShaderResource objects for a \see spRenderingPipeline.
struct spResourceLayoutDescription : public ezHashableStruct<spResourceLayoutDescription>
{
  /// \brief An array of \see spResourceLayoutElementDescription objects, describing the properties of each resource
  /// element in the \see spResourceLayout.
  ezDynamicArray<spResourceLayoutElementDescription> m_Elements;
};

class SP_RHI_DLL spResourceLayout : public spDeviceResource
{
  friend class spDeviceResourceFactory;

public:
  EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<spResourceLayoutElementDescription>& GetElements() const { return m_Description.m_Elements; }

protected:
  explicit spResourceLayout(spResourceLayoutDescription description);

  spResourceLayoutDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spResourceLayout);
