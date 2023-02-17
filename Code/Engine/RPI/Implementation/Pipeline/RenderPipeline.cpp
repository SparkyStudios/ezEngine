#include <RPI/RPIPCH.h>

#include <RPI/Pipeline/RenderPass.h>
#include <RPI/Pipeline/RenderPipeline.h>

spRenderPipeline::spRenderPipeline(spRenderGraphResourcesTable&& resources)
  : m_PipelineResources(resources)
{
}

void spRenderPipeline::Execute(spRenderingContext* pContext)
{
  for (const auto& pass : m_OrderedPasses)
  {
    const ezUniquePtr<spRenderPass>* pPass = m_Passes.GetValue(pass);

    BeginPass(pPass->Borrow(), pContext);
    ExecutePass(pPass->Borrow(), pContext);
    EndPass(pPass->Borrow(), pContext);
  }
}

void spRenderPipeline::BeginPass(spRenderPass* pPass, spRenderingContext* pContext)
{
  m_PassEvents.Broadcast({spRenderPassEvent::Type::BeforePass, this, pPass});
}

void spRenderPipeline::ExecutePass(spRenderPass* pPass, spRenderingContext* pContext)
{
  pPass->Execute(m_PipelineResources, pContext);
}

void spRenderPipeline::EndPass(spRenderPass* pPass, spRenderingContext* pContext)
{
  m_PassEvents.Broadcast({spRenderPassEvent::Type::AfterPass, this, pPass});
}

void spRenderPipeline::AddPass(ezHashedString sName, ezUniquePtr<spRenderPass>&& pPass)
{
  m_Passes.Insert(sName, std::move(pPass));
  m_OrderedPasses.PushBack(sName);
}

spRenderPass::spRenderPass(ExecuteCallback callback)
  : m_Callback(std::move(callback))
{
}

void spRenderPass::Execute(const spRenderGraphResourcesTable& resources, spRenderingContext* context)
{
  m_Callback(resources, context, m_PassData);
}

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Pipeline_RenderPipeline);
