#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Core.h>

#include <Foundation/Containers/IdTable.h>

/// \brief Represents an invalid joint index.
#define spSkeletonInvalidJointIndex static_cast<ezUInt16>(0xFFFFu)

namespace RPI
{
  class spRenderGraphResource;

  typedef ezIdTable<RHI::spResourceHandleId, spRenderGraphResource> spRenderGraphResourcesTable;

  typedef ezGenericId<24, 8> spRenderViewId;

  class spRenderViewHandle
  {
    EZ_DECLARE_HANDLE_TYPE(spRenderViewHandle, spRenderViewId);

    friend class ezRenderWorld;
  };

  enum SP_RPI_CONSTANTS : ezUInt8
  {
    /// \brief The maximum number of lod a mesh can have.
    SP_RPI_MAX_LOD_COUNT = 5,

    SP_RPI_MAX_MIP_COUNT = 12,
  };
} // namespace RPI
