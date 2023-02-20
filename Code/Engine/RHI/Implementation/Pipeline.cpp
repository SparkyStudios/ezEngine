#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Pipeline.h>

#pragma region spPipeline

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spPipeline, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezSharedPtr<spResourceLayout> spPipeline::GetResourceLayout(ezUInt32 uiSlot) const
{
  EZ_ASSERT_DEV(uiSlot < m_ResourceLayouts.GetCount(), "Invalid slot index {0}, values are in the range [0, {1}].", uiSlot, m_ResourceLayouts.GetCount());
  return m_ResourceLayouts[uiSlot];
}

#pragma endregion

#pragma region spComputePipeline

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spComputePipeline, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spComputePipeline::spComputePipeline(spComputePipelineDescription description)
  : spPipeline()
  , m_Description(std::move(description))
{
}

#pragma endregion

#pragma region spGraphicPipeline

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spGraphicPipeline, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spGraphicPipeline::spGraphicPipeline(spGraphicPipelineDescription description)
  : spPipeline()
  , m_Description(std::move(description))
{
  m_hShaderProgram = m_Description.m_ShaderPipeline.m_hShaderProgram;
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Pipeline);
