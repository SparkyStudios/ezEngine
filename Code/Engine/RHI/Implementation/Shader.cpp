#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Shader.h>

#pragma region spShaderPipeline

spShaderPipeline::spShaderPipeline(
  ezDynamicArray<spResourceHandle> inputLayouts,
  spResourceHandle hVertexShader,
  spResourceHandle hPixelShader,
  spResourceHandle hGeometryShader,
  spResourceHandle hDomainShader,
  spResourceHandle hHullShader)
  : ezHashableStruct<spShaderPipeline>()
  , m_InputLayouts(std::move(inputLayouts))
  , m_hVertexShader(hVertexShader)
  , m_hPixelShader(hPixelShader)
  , m_hGeometryShader(hGeometryShader)
  , m_hHullShader(hHullShader)
  , m_hDomainShader(hDomainShader)
{
  const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<spDevice>();
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hVertexShader));
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hPixelShader));

  if (!m_hGeometryShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hGeometryShader));

  if (!m_hDomainShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hDomainShader));

  if (!m_hHullShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hHullShader));
}

spShaderPipeline::spShaderPipeline(
  ezDynamicArray<spResourceHandle> inputLayouts,
  spResourceHandle hComputeShader)
  : ezHashableStruct<spShaderPipeline>()
  , m_InputLayouts(std::move(inputLayouts))
  , m_hComputeShader(hComputeShader)
{
  const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<spDevice>();
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hComputeShader));
}

spShaderPipeline::~spShaderPipeline()
{
  const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<spDevice>();

  if (!m_hVertexShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hVertexShader));

  if (!m_hPixelShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hPixelShader));

  if (!m_hGeometryShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hGeometryShader));

  if (!m_hDomainShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hDomainShader));

  if (!m_hHullShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hHullShader));

  if (!m_hComputeShader.IsInvalidated())
    EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hComputeShader));
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Shader);
