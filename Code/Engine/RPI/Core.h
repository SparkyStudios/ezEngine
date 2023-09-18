#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Core.h>

#include <Foundation/Containers/IdTable.h>

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
} // namespace RPI
