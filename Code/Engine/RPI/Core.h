#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Core.h>

#include <Foundation/Containers/IdTable.h>

class spRenderGraphResource;

typedef ezIdTable<spResourceHandleId, spRenderGraphResource> spRenderGraphResourcesTable;

typedef ezGenericId<24, 8> spRenderViewId;

enum SP_RPI_CONSTANTS : ezUInt8
{
  /// \brief The maximum number of lod a mesh can have.
  SP_RPI_MAX_LOD_COUNT = 5
};

class spRenderViewHandle
{
  EZ_DECLARE_HANDLE_TYPE(spRenderViewHandle, spRenderViewId);

  friend class ezRenderWorld;
};
