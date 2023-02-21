#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Core/RenderingContext.h>
#include <RPI/Graph/RenderGraph.h>

class SP_RPI_DLL spRenderPass
{
  EZ_DISALLOW_COPY_AND_ASSIGN(spRenderPass);

public:
  typedef ezDelegate<void(const spRenderGraphResourcesTable&, spRenderingContext*, ezVariant&)> ExecuteCallback;
  typedef ezDelegate<void(const spRenderGraphResourcesTable&, ezVariant&)> CleanUpCallback;

  spRenderPass(ExecuteCallback executeCallback, CleanUpCallback cleanUpCallback);

  template <typename T>
  EZ_ALWAYS_INLINE void SetData(const T& data)
  {
    m_PassData = data;
  }

  virtual void Execute(const spRenderGraphResourcesTable& resources, spRenderingContext* context);

  virtual void CleanUp(const spRenderGraphResourcesTable& resources);

private:
  ExecuteCallback m_ExecuteCallback;
  CleanUpCallback m_CleanUpCallback;

  ezVariant m_PassData;
};
