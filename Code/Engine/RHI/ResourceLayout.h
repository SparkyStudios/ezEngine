#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Resource.h>

/// \brief Describes a single element in a resource layout.
struct SP_RHI_DLL spResourceLayoutElementDescription : public ezHashableStruct<spResourceLayoutElementDescription>
{
  ezHashedString m_sName;
  ezEnum<spShaderResourceType> m_eType;
  ezBitflags<spShaderStage> m_eShaderStage;
  ezBitflags<spResourceLayoutElementOptions> m_eOptions;
};

/// \brief Describes the layout of \see spShaderResource objects for a \see spRenderingPipeline.
struct SP_RHI_DLL spResourceLayoutDescription : public ezHashableStruct<spResourceLayoutDescription>
{
  /// \brief An array of \see spResourceLayoutElementDescription objects, describing the properties of each resource
  /// element in the \see spResourceLayout.
  ezDynamicArray<spResourceLayoutElementDescription> m_Elements;
};

class SP_RHI_DLL spResourceLayout : public spDeviceResource
{
public:
  // spResoureLayout
  EZ_NODISCARD EZ_ALWAYS_INLINE ezDynamicArray<spResourceLayoutElementDescription> GetElements() const { return m_Elements; }

protected:
  ezDynamicArray<spResourceLayoutElementDescription> m_Elements;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spResourceLayout);
