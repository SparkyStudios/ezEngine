#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/ResourceLayout.h>

/// \brief Describes a \see spResourceSet, for creation using a \see spDeviceResourceFactory.
struct spResourceSetDescription : public ezHashableStruct<spResourceSetDescription>
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
  friend class spDeviceResourceFactory;

public:
  EZ_NODISCARD const spResourceHandle& GetLayout() const { return m_Description.m_hResourceLayout; }
  EZ_NODISCARD ezDynamicArray<spResourceHandle> GetElements() const { return m_Description.m_BoundResources; }

protected:
  explicit spResourceSet(spResourceSetDescription description);

  spResourceSetDescription m_Description;
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spResourceSet);
