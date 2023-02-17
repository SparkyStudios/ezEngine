#include <RPI/RPIPCH.h>

#include <RPI/Core/RenderingContext.h>

spRenderingContext::spRenderingContext(spDevice* pDevice)
  : m_pDevice(pDevice)
{
}

spRenderingContext::~spRenderingContext()
{
  m_pCommandList.Clear();
}

void spRenderingContext::BeginFrame()
{
  m_pDevice->BeginFrame();

  m_pCommandList->Begin();
}

void spRenderingContext::EndFrame()
{
  m_pCommandList->End();

  m_pDevice->SubmitCommandList(m_pCommandList);
  m_pDevice->EndFrame();
}
