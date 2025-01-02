// Copyright (c) 2024-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Resources/MaterialResource.h>
#include <RPI/Resources/ShaderResource.h>

#include <RHI/Shader.h>

#include <slang-com-ptr.h>

EZ_DEFINE_AS_POD_TYPE(slang::PreprocessorMacroDesc);

namespace RPI
{
  /// \brief Describes a single shader compilation setup.
  struct spShaderCompilerSetup
  {
    /// \brief The shader resource this setup applies to.
    spShaderResourceHandle m_hShaderResource;

    /// \brief The shader stage to generate.
    ezEnum<RHI::spShaderStage> m_eStage;

    /// \brief The list of predefined macros for this shader.
    ezDynamicArray<slang::PreprocessorMacroDesc> m_PredefinedMacros;

    /// \brief The list of specialization constants for this shader.
    ezDynamicArray<RHI::spShaderSpecializationConstant> m_SpecializationConstants;

    /// \brief The material resource this shader is compiled for.
    /// This is used to link the shader with the material resource. If no material resource is needed,
    /// this should be set to an invalid resource handle.
    spRootMaterialResourceHandle m_hRootMaterialResource;

    /// \brief Calculates a hash value for this shader compilation setup.
    [[nodiscard]] ezUInt32 CalculateHash() const;
  };

  /// \brief Describes a single root material compilation setup.
  struct spRootMaterialCompilerSetup
  {
    /// \brief The path to the root material file.
    ezStringView m_sRootMaterialPath;

    /// \brief The list of predefined macros for this root material.
    ezDynamicArray<slang::PreprocessorMacroDesc> m_PredefinedMacros;
  };

  class SP_RPI_DLL spShaderManager
  {
    EZ_DECLARE_SINGLETON(spShaderManager);

  public:
    spShaderManager();
    ~spShaderManager();

    /// \brief Compiles a shader based on the given compiler setup.
    /// \param compilerSetup The shader compilation setup for which to compile the shader.
    ezSharedPtr<RHI::spShader> CompileShader(const spShaderCompilerSetup& compilerSetup);

    /// \brief Compiles a root material based on the given compiler setup.
    /// \param compilerSetup The root material compilation setup for which to compile the root material.
    /// \param out_ir The intermediate representation (IR) of the compiled root material.
    void CompileRootMaterial(const spRootMaterialCompilerSetup& compilerSetup, Slang::ComPtr<slang::IBlob>& out_ir);

  private:
    void CacheShaderKernel(ezUInt32 uiHash, ezSharedPtr<RHI::spShader> pShader);

    /// \brief Creates a Slang shader module from the set of shader specialization constants.
    /// \param pSession The Slang shader compilation session.
    /// \param compilerSetup The shader compilation setup for which to create the Slang shader module.
    /// \return A pointer to the Slang shader module for the given shader specialization constants, or nullptr if compilation failed.
    static Slang::ComPtr<slang::IModule> CreateSpecializationConstantsModule(Slang::ComPtr<slang::ISession> pSession, const spShaderCompilerSetup& compilerSetup);

    /// \brief Creates a Slang shader module for the given material resource.
    /// \param pSession The Slang shader compilation session.
    /// \param compilerSetup The shader compilation setup for which to create the Slang shader module.
    /// \return A pointer to the Slang shader module for the given material resource, or nullptr if compilation failed.
    static Slang::ComPtr<slang::IModule> CreateMaterialModule(Slang::ComPtr<slang::ISession> pSession, const spShaderCompilerSetup& compilerSetup);

    /// \brief Gets the entry point for the given shader stage from the given Slang shader module.
    /// \param pModule The Slang shader module.
    /// \param eStage The shader stage for which to get the entry point.
    /// \return A pointer to the Slang entry point for the given shader stage, or nullptr if not found.
    static Slang::ComPtr<slang::IEntryPoint> GetEntryPointForStage(Slang::ComPtr<slang::IModule> pModule, ezEnum<RHI::spShaderStage> eStage);

    static void LoadCoreModules(Slang::ComPtr<slang::ISession> pSession);

    /// \brief Creates a new Slang shader compilation session.
    /// \param predefinedMacros The list of predefined macros to use in the session.
    [[nodiscard]] Slang::ComPtr<slang::ISession> CreateSlangSession(const ezArrayPtr<const slang::PreprocessorMacroDesc>& predefinedMacros) const;

    Slang::ComPtr<slang::IGlobalSession> m_pGlobalSession{nullptr};
    ezArrayMap<ezUInt32, ezSharedPtr<RHI::spShader>> m_ShaderKernelCache;
  };
} // namespace RPI
