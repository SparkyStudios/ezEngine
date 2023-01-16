#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/ResourceLayout.h>

/// \brief Describes a \see spResourceSet, for creation using a \see spDeviceResourceFactory.
struct SP_RHI_DLL spResourceSetDescription : public ezHashableStruct<spResourceSetDescription>
{
  /// \brief The handle to the \see spResourceLayout resource describing the number and kind of resources
  /// used by this resource set.
  spResourceHandle m_hResourceLayout;

  /// \brief An array of \see spShaderResource objects.
  /// The number and type of resources must match those specified in the \see spResourceLayout resource.
  ezDynamicArray<spResourceHandle> m_BoundResources;
};

class SP_RHI_DLL spResourceSet : public spDeviceResource
{
public:
  EZ_NODISCARD EZ_ALWAYS_INLINE const spResourceHandle& GetLayout() const { return m_hLayout; }
  EZ_NODISCARD EZ_ALWAYS_INLINE ezDynamicArray<spResourceHandle> GetElements() const { return m_Elements; }

protected:
  spResourceHandle m_hLayout;
  ezDynamicArray<spResourceHandle> m_Elements;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spResourceSet);
