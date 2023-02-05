#include <RHI/RHIPCH.h>

#include <RHI/Pipeline.h>

#pragma region spComputePipeline

spComputePipeline::spComputePipeline(spComputePipelineDescription description)
  : spPipeline()
  , m_Description(std::move(description))
{
}

#pragma endregion

#pragma region spGraphicPipeline

spGraphicPipeline::spGraphicPipeline(spGraphicPipelineDescription description)
  : spPipeline()
  , m_Description(std::move(description))
{
  m_hShaderProgram = m_Description.m_ShaderPipeline.m_hShaderProgram;
}

#pragma endregion
