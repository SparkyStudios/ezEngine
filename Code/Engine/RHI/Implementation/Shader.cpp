#include <RHI/RHIPCH.h>

#include <RHI/Device.h>
#include <RHI/Shader.h>

#pragma region spShaderProgram

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderProgram, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

#pragma endregion

#pragma region spShader

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShader, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

spShader::spShader(spShaderDescription description)
  : m_Description(std::move(description))
{
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Shader);
