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
  /// \brief The data type of a specialization constant.
  struct spShaderSpecializationConstantType
  {
    typedef ezUInt8 StorageType;

    enum Enum : StorageType
    {
      /// \brief A boolean.
      Bool,

      /// \brief A 16-bit unsigned integer.
      UInt16,

      /// \brief A 16-bit signed integer.
      Int16,

      /// \brief A 32-bit unsigned integer.
      UInt32,

      /// \brief A 32-bit signed integer.
      Int32,

      /// \brief A 64-bit unsigned integer.
      UInt64,

      /// \brief A 64-bit signed integer.
      Int64,

      /// \brief A 32-bit floating-point value.
      Float,

      /// \brief A 64-bit floating-point value.
      Double,

      Default = Bool
    };
  };

  /// \brief Describes a single shader specialization constant.
  struct spShaderSpecializationConstant : ezHashableStruct<spShaderSpecializationConstant>
  {
    /// \brief The specialization constant name, as defined in the shader.
    ezHashedString m_sName;

    /// \brief The type of data stored in the specialization constant. Must be a scalar type.
    ezEnum<spShaderSpecializationConstantType> m_eType{spShaderSpecializationConstantType::Default};

    /// \brief A 8-byte block containing the value of the specialization constant. This is treated as an
    /// untyped buffer and it's interpreted according to the type.
    ezUInt64 m_uiValue{0};

    /// \brief Constructs a new instance of spShaderSpecializationConstant.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] eType The type of data stored in the specialization constant.
    /// \param [in] uiData The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, const ezEnum<spShaderSpecializationConstantType>& eType, ezUInt64 uiData)
      : ezHashableStruct<spShaderSpecializationConstant>()
      , m_sName(sName)
      , m_eType(eType)
      , m_uiValue(uiData)
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for a boolean value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] bValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, bool bValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::Bool, Store(bValue ? 1u : 0u))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for an unsigned short integer value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] uiValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, ezUInt16 uiValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::UInt16, Store(uiValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for a short integer value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] iValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, ezInt16 iValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::Int16, Store(iValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for an unsigned integer value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] uiValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, ezUInt32 uiValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::UInt32, Store(uiValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for an integer value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] iValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, ezInt32 iValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::Int32, Store(iValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for an unsigned long integer value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] uiValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, ezUInt64 uiValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::UInt64, Store(uiValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for a long integer value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] iValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, ezInt64 iValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::Int64, Store(iValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for a float value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] fValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, float fValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::Float, Store(fValue))
    {
    }

    /// \brief Constructs a new instance of spShaderSpecializationConstant for a double value.
    /// \param [in] sName The name of the specialization constant.
    /// \param [in] dValue The value of the specialization constant.
    spShaderSpecializationConstant(ezHashedString sName, double dValue)
      : spShaderSpecializationConstant(sName, spShaderSpecializationConstantType::Double, Store(dValue))
    {
    }

    template <typename T>
    static T Get(ezUInt64 uiValue)
    {
      T value;
      ezMemoryUtils::RawByteCopy(&value, &uiValue, sizeof(T));
      return value;
    }

  private:
    template <typename T>
    static ezUInt64 Store(T value)
    {
      ezUInt64 uiValue = 0;
      ezMemoryUtils::RawByteCopy(&uiValue, &value, sizeof(T));
      return uiValue;
    }
  };

  struct spShaderCompilerSetup
  {
    ezEnum<RHI::spShaderStage> m_eStage;
    ezDynamicArray<slang::PreprocessorMacroDesc> m_PredefinedMacros;
    ezDynamicArray<spShaderSpecializationConstant> m_SpecializationConstants;

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
        spShaderSpecializationConstantType::Enum eType = constant.m_eType;
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
