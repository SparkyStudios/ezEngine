#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderingContext.h>

using namespace RHI;

spRenderingContext::spRenderingContext(spDevice* pDevice)
  : m_pDevice(pDevice)
{
}

spRenderingContext::~spRenderingContext()
{
  m_pCommandList.Clear();
}

void spRenderingContext::SetCommandList(ezSharedPtr<spCommandList> pCommandList)
{
  if (m_pCommandList == pCommandList)
    return;

  m_pCommandList = pCommandList;
  m_pCommandList->Reset();
}

void spRenderingContext::BeginFrame()
{
  m_pCommandList->Begin();
}

void spRenderingContext::EndFrame(ezSharedPtr<spFence> pFence)
{
  m_pCommandList->End();

  m_pDevice->SubmitCommandList(m_pCommandList, pFence);
}

void spRenderingContext::Reset()
{
  m_pCommandList->Reset();
}
