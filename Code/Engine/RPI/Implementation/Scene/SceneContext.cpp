#include <RPI/RPIPCH.h>

#include <RPI/Scene/SceneContext.h>

using namespace RHI;

spSceneContext::spSceneContext(spDevice* pDevice)
  : m_pDevice(pDevice)
{
  m_pCommandList = pDevice->GetResourceFactory()->CreateCommandList(spCommandListDescription());
  m_pFence = pDevice->GetResourceFactory()->CreateFence(spFenceDescription(false));

  m_pRenderingContext = EZ_NEW(pDevice->GetAllocator(), spRenderingContext, pDevice);
}

void spSceneContext::AddPipeline(spRenderPipeline* pPipeline)
{
  m_RenderPipelines.PushBack(pPipeline);
}

void spSceneContext::BeginFrame()
{
  m_pDevice->BeginFrame();

  m_pRenderingContext->SetCommandList(m_pCommandList);
  m_pRenderingContext->BeginFrame();
}

void spSceneContext::Draw()
{
  for (auto it = m_RenderPipelines.GetIterator(); it.IsValid(); it.Next())
    (*it)->Execute(m_pRenderingContext.Borrow());
}

void spSceneContext::EndFrame()
{
  m_pRenderingContext->EndFrame();
  m_pRenderingContext->Reset();

  m_pDevice->EndFrame();
  m_pDevice->RaiseFence(m_pFence);
}

void spSceneContext::WaitForIdle()
{
  m_pDevice->WaitForFence(m_pFence);
  m_pDevice->WaitForIdle();

  m_pDevice->ResetFence(m_pFence);
}
