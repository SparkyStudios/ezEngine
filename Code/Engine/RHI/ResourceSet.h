#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/ResourceLayout.h>

/// \brief Describes a \a spResourceSet, for creation using a \a spDeviceResourceFactory.
struct spResourceSetDescription : ezHashableStruct<spResourceSetDescription>
{
  /// \brief The handle to the \a spResourceLayout resource describing the number and kind of resources
  /// used by this resource set.
  spResourceHandle m_hResourceLayout;

  /// \brief An array of \a spShaderResource objects.
  /// The number and type of resources must match those specified in the \a spResourceLayout resource.
  ezDynamicArray<spResourceHandle> m_BoundResources;
};

class SP_RHI_DLL spResourceSet : public spDeviceResource
{
  EZ_ADD_DYNAMIC_REFLECTION(spResourceSet, spDeviceResource);

  friend class spDeviceResourceFactory;

public:
  EZ_NODISCARD EZ_ALWAYS_INLINE const spResourceHandle& GetLayout() const { return m_Description.m_hResourceLayout; }
  EZ_NODISCARD EZ_ALWAYS_INLINE const ezDynamicArray<spResourceHandle>& GetResources() const { return m_Description.m_BoundResources; }
  EZ_NODISCARD EZ_ALWAYS_INLINE spResourceHandle GetResource(ezUInt32 uiIndex) const { return m_Description.m_BoundResources[uiIndex]; }

  EZ_NODISCARD EZ_ALWAYS_INLINE spResourceSetDescription GetDescription() const { return m_Description; }

protected:
  explicit spResourceSet(spResourceSetDescription description);

  spResourceSetDescription m_Description;
};
