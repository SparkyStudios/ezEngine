#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Core.h>
#include <RPI/Core/RenderingContext.h>

class spRenderPipeline;
class spRenderPass;

/// \brief An event triggered before the start and after the end of a \a spRenderPass.
struct spRenderPassEvent
{
  enum class Type
  {
    BeforePass,
    AfterPass,
  };

  Type m_Type;
  spRenderPipeline* m_pPipeline = nullptr;
  spRenderPass* m_pPass = nullptr;
};

class SP_RPI_DLL spRenderPipeline : public ezRefCounted
{
  EZ_DISALLOW_COPY_AND_ASSIGN(spRenderPipeline);

public:
  explicit spRenderPipeline(spRenderGraphResourcesTable&& resources);

  void Execute(spRenderingContext* pContext);

  void BeginPass(spRenderPass* pPass, spRenderingContext* pContext);
  void ExecutePass(spRenderPass* pPass, spRenderingContext* pContext);
  void EndPass(spRenderPass* pPass, spRenderingContext* pContext);

  void AddPass(ezHashedString sName, ezUniquePtr<spRenderPass>&& pPass);

  void RemovePass(ezHashedString sName);

  bool TryGetPass(ezHashedString sName, spRenderPass*& pPass);

  spRenderPass* GetPass(ezHashedString sName);

private:
  ezEvent<const spRenderPassEvent&, ezMutex> m_PassEvents;

  spRenderGraphResourcesTable m_PipelineResources;

  ezHashTable<ezHashedString, ezUniquePtr<spRenderPass>> m_Passes;
  ezDynamicArray<ezHashedString> m_OrderedPasses;
};
