#include <RPI/RPIPCH.h>

#include <RPI/Graph/RenderGraph.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRenderGraphNode, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Pipeline_Graph_RenderGraphNode);
