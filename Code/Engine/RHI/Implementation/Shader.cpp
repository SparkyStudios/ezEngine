#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Shader.h>

#pragma region spShaderPipeline

spShaderPipeline::spShaderPipeline(ezDynamicArray<spResourceHandle> inputLayouts, spResourceHandle hShaderPipeline)
  : ezHashableStruct<spShaderPipeline>()
  , m_InputLayouts(std::move(inputLayouts))
  , m_hShaderProgram(hShaderPipeline)
{
  const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<spDevice>();
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->IncrementResourceRef(m_hShaderProgram));
}

spShaderPipeline::~spShaderPipeline()
{
  const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<spDevice>();
  EZ_IGNORE_UNUSED(pDevice->GetResourceManager()->DecrementResourceRef(m_hShaderProgram));
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Shader);
