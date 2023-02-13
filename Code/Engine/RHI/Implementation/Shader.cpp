#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Shader.h>

#pragma region spShaderPipeline

spShaderPipeline::spShaderPipeline(ezDynamicArray<spResourceHandle> inputLayouts, spResourceHandle hShaderProgram)
  : ezHashableStruct<spShaderPipeline>()
  , m_InputLayouts(std::move(inputLayouts))
  , m_hShaderProgram(hShaderProgram)
{
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Shader);
