#pragma once

#include <RPI/RPIDLL.h>

#include <RHI/Core.h>

#include <Foundation/Containers/IdTable.h>

class spRenderGraphResource;

typedef ezIdTable<spResourceHandleId, spRenderGraphResource> spRenderGraphResourcesTable;
