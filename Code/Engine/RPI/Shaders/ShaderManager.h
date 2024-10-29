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

#include <RAI/Resources/ShaderResource.h>

#include <RHI/Shader.h>

#include <slang-com-ptr.h>

EZ_DEFINE_AS_POD_TYPE(slang::PreprocessorMacroDesc);

namespace RPI
{
  struct spShaderCompilerSetup
  {
    ezEnum<RHI::spShaderStage> m_eStage;
    ezDynamicArray<slang::PreprocessorMacroDesc> m_PredefinedMacros;
    ezDynamicArray<RHI::spShaderSpecializationConstant> m_SpecializationConstants;

    ezUInt32 CalculateHash() const
    {
      ezUInt32 uiHash = 0;
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32(&m_eStage, sizeof(m_eStage), uiHash));

      for (const auto& macro : m_PredefinedMacros)
      {
        uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32String(macro.name, uiHash));
        uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32String(macro.value, uiHash));
      }

      for (const auto& constant : m_SpecializationConstants)
      {
        uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32String(constant.m_sName, uiHash));
        RHI::spShaderSpecializationConstantType::Enum eType = constant.m_eType;
        uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32(&eType, sizeof(eType), uiHash));
        uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32(&constant.m_uiValue, sizeof(constant.m_uiValue), uiHash));
      }

      return uiHash;
    }
  };

  class SP_RPI_DLL spShaderManager
  {
    EZ_DECLARE_SINGLETON(spShaderManager);

  public:
    spShaderManager();
    ~spShaderManager();

    ezSharedPtr<RHI::spShader> CompileShader(RAI::spShaderResourceHandle hShaderResource, spShaderCompilerSetup& ref_compilerSetup, slang::IComponentType** out_pShaderProgram = nullptr);

  private:
    void CacheShaderKernel(ezUInt32 uiHash, ezSharedPtr<RHI::spShader> pShader);

    /// \brief Creates a Slang shader code from the set of shader specialization constants.
    static Slang::ComPtr<slang::IModule> CreateSpecializationConstantsCode(Slang::ComPtr<slang::ISession> pSession, const spShaderCompilerSetup& compilerSetup);

    /// \brief Creates a new Slang shader compilation session.
    [[nodiscard]] Slang::ComPtr<slang::ISession> CreateSlangSession(spShaderCompilerSetup& ref_compilerSetup) const;

    static Slang::ComPtr<slang::IEntryPoint> GetEntryPointForStage(Slang::ComPtr<slang::IModule> pModule, ezEnum<RHI::spShaderStage> eStage);

    Slang::ComPtr<slang::IGlobalSession> m_pGlobalSession{nullptr};
    ezArrayMap<ezUInt32, ezSharedPtr<RHI::spShader>> m_ShaderKernelCache;
  };
} // namespace RPI
