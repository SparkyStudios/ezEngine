#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Core/RenderingContext.h>
#include <RPI/Pipeline/Graph/RenderGraph.h>

class SP_RPI_DLL spRenderPass
{
  EZ_DISALLOW_COPY_AND_ASSIGN(spRenderPass);

public:
  typedef ezDelegate<void(const spRenderGraphResourcesTable&, spRenderingContext*, ezVariant&)> ExecuteCallback;

  spRenderPass(ExecuteCallback callback);

  template <typename T>
  EZ_ALWAYS_INLINE void SetData(const T& data)
  {
    m_PassData = data;
  }

  void Execute(const spRenderGraphResourcesTable& resources, spRenderingContext* context);

private:
  ExecuteCallback m_Callback;
  ezVariant m_PassData;
};
