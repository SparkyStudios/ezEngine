#include <RHID3D11/RHID3D11PCH.h>

#include <RHI/Input.h>

#include <RHID3D11/Core.h>
#include <RHID3D11/Device.h>
#include <RHID3D11/Pipeline.h>
#include <RHID3D11/ResourceLayout.h>
#include <RHID3D11/ResourceManager.h>
#include <RHID3D11/Shader.h>


#pragma region spComputePipelineD3D11

void spComputePipelineD3D11::ReleaseResource()
{
  EZ_IGNORE_UNUSED(m_pDevice->GetResourceManager()->DecrementResourceRef(m_Description.m_hComputeShader));

  for (ezUInt32 i = 0, l = m_Description.m_ResourceLayouts.GetCount(); i < l; ++i)
    EZ_IGNORE_UNUSED(m_pDevice->GetResourceManager()->DecrementResourceRef(m_Description.m_ResourceLayouts[i]));

  m_bReleased = true;
}

bool spComputePipelineD3D11::IsReleased() const
{
  return m_bReleased;
}

spComputePipelineD3D11::spComputePipelineD3D11(spDeviceD3D11* pDevice, const spComputePipelineDescription& description)
  : spComputePipeline(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();

  const auto* shader = m_pDevice->GetResourceManager()->GetResource<spShaderD3D11>(description.m_hComputeShader);
  EZ_ASSERT_DEV(shader != nullptr, "Invalid compute shader resource {0}", description.m_hComputeShader.GetInternalID().m_Data);

  EZ_IGNORE_UNUSED(shader->AddRef());
  m_pComputeShader = static_cast<ID3D11ComputeShader*>(shader->GetD3D11Shader());

  m_ResourceLayouts.Reserve(description.m_ResourceLayouts.GetCount());
  for (ezUInt32 i = 0, l = description.m_ResourceLayouts.GetCount(); i < l; ++i)
  {
    auto* layout = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutD3D11>(description.m_ResourceLayouts[i]);
    EZ_ASSERT_DEV(layout != nullptr, "Invalid resource layout handle {0}", description.m_hComputeShader.GetInternalID().m_Data);

    EZ_IGNORE_UNUSED(layout->AddRef());
    m_ResourceLayouts[i] = layout;
  }

  m_bReleased = false;
}

#pragma endregion

#pragma region spGraphicPipelineD3D11

void spGraphicPipelineD3D11::ReleaseResource()
{
  EZ_IGNORE_UNUSED(m_pDevice->GetResourceManager()->DecrementResourceRef(m_hShaderProgram));

  for (ezUInt32 i = 0, l = m_Description.m_ResourceLayouts.GetCount(); i < l; ++i)
    EZ_IGNORE_UNUSED(m_pDevice->GetResourceManager()->DecrementResourceRef(m_Description.m_ResourceLayouts[i]));

  for (ezUInt32 i = 0, l = m_Description.m_ShaderPipeline.m_InputLayouts.GetCount(); i < l; ++i)
    EZ_IGNORE_UNUSED(m_pDevice->GetResourceManager()->DecrementResourceRef(m_Description.m_ShaderPipeline.m_InputLayouts[i]));

  m_bReleased = true;
}

bool spGraphicPipelineD3D11::IsReleased() const
{
  return m_bReleased;
}

spGraphicPipelineD3D11::spGraphicPipelineD3D11(spDeviceD3D11* pDevice, const spGraphicPipelineDescription& description)
  : spGraphicPipeline(description)
{
  m_pDevice = pDevice;
  m_pD3D11Device = pDevice->GetD3D11Device();

  const auto* pShaderProgram = m_pDevice->GetResourceManager()->GetResource<spShaderProgramD3D11>(m_hShaderProgram);
  EZ_ASSERT_DEV(pShaderProgram != nullptr, "Invalid shader program resource {0}", m_hShaderProgram.GetInternalID().m_Data);

  EZ_IGNORE_UNUSED(pShaderProgram->AddRef());

  ezByteArrayPtr vertexShaderByteCode;
  if (auto* pShader = pShaderProgram->GetVertexShader(); pShader != nullptr)
  {
    m_pVertexShader = static_cast<ID3D11VertexShader*>(pShader->GetD3D11Shader());
    vertexShaderByteCode = pShader->GetShaderByteCode();
  }

  EZ_ASSERT_DEV(m_pVertexShader != nullptr, "Cannot create a graphic pipeline without a vertex shader.");

  if (auto* pShader = pShaderProgram->GetGeometryShader(); pShader != nullptr)
  {
    m_pGeometryShader = static_cast<ID3D11GeometryShader*>(pShader->GetD3D11Shader());
  }

  if (auto* pShader = pShaderProgram->GetHullShader(); pShader != nullptr)
  {
    m_pHullShader = static_cast<ID3D11HullShader*>(pShader->GetD3D11Shader());
  }

  if (auto* pShader = pShaderProgram->GetDomainShader(); pShader != nullptr)
  {
    m_pDomainShader = static_cast<ID3D11DomainShader*>(pShader->GetD3D11Shader());
  }

  if (auto* pShader = pShaderProgram->GetPixelShader(); pShader != nullptr)
  {
    m_pPixelShader = static_cast<ID3D11PixelShader*>(pShader->GetD3D11Shader());
  }

  ezUInt32 uiVertexBuffersCount = description.m_ShaderPipeline.m_InputLayouts.GetCount();

  ezDynamicArray<spInputLayoutDescription> inputLayouts;

  inputLayouts.Reserve(uiVertexBuffersCount);
  m_InputLayouts.Reserve(uiVertexBuffersCount);
  for (ezUInt32 i = 0, l = uiVertexBuffersCount; i < l; ++i)
  {
    auto* layout = m_pDevice->GetResourceManager()->GetResource<spInputLayout>(description.m_ShaderPipeline.m_InputLayouts[i]);
    EZ_ASSERT_DEV(layout != nullptr, "Invalid input layout handle {0}", description.m_ShaderPipeline.m_InputLayouts[i].GetInternalID().m_Data);

    EZ_IGNORE_UNUSED(layout->AddRef());
    m_InputLayouts[i] = layout;

    inputLayouts[i] = layout->GetDescription();
  }

  pDevice->GetD3D11ResourceManager()->GetPipelineResources(
    description.m_RenderingState.m_BlendState,
    description.m_RenderingState.m_DepthState,
    description.m_RenderingState.m_StencilState,
    description.m_RenderingState.m_RasterizerState,
    description.m_Output.m_eSampleCount != spTextureSampleCount::None,
    inputLayouts.GetArrayPtr(),
    vertexShaderByteCode,
    m_pBlendState,
    m_pDepthStencilState,
    m_pRasterizerState,
    m_pInputLayout);

  m_BlendFactor = description.m_RenderingState.m_BlendState.m_BlendColor;
  m_StencilRef = description.m_RenderingState.m_StencilState.m_uiReference;
  m_PrimitiveTopology = spToD3D11(description.m_ePrimitiveTopology);

  ezUInt32 uiResourceLayoutsCount = description.m_ResourceLayouts.GetCount();

  m_ResourceLayouts.Reserve(uiResourceLayoutsCount);
  for (ezUInt32 i = 0, l = uiResourceLayoutsCount; i < l; ++i)
  {
    auto* layout = m_pDevice->GetResourceManager()->GetResource<spResourceLayoutD3D11>(description.m_ResourceLayouts[i]);
    EZ_ASSERT_DEV(layout != nullptr, "Invalid resource layout handle {0}", description.m_ResourceLayouts[i].GetInternalID().m_Data);

    EZ_IGNORE_UNUSED(layout->AddRef());
    m_ResourceLayouts[i] = layout;
  }

  if (uiVertexBuffersCount > 0)
  {
    m_VertexStrides.Reserve(uiVertexBuffersCount);
    for (ezUInt32 i = 0; i < uiVertexBuffersCount; ++i)
    {
      m_VertexStrides[i] = inputLayouts[i].m_uiStride;
    }
  }

  m_bReleased = false;
}

#pragma endregion
