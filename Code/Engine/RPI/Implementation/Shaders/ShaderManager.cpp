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

#include <RPI/RPIPCH.h>

#include <RPI/Shaders/ShaderManager.h>

#include <RHI/Device.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>

#include <slang-com-helper.h>

struct spSlangByteBlob final : slang::IBlob
{
  SLANG_REF_OBJECT_IUNKNOWN_ALL

  explicit spSlangByteBlob(const ezByteArrayPtr& pData)
    : m_pData(pData)
    , m_bOwned(false)
  {
  }

  explicit spSlangByteBlob(const ezStringBuilder& sbData)
  {
    m_pData = EZ_DEFAULT_NEW_ARRAY(ezUInt8, sbData.GetElementCount());
    ezMemoryUtils::RawByteCopy(m_pData.GetPtr(), sbData.GetData(), sbData.GetElementCount());
    m_bOwned = true;
  }

  virtual ~spSlangByteBlob()
  {
    if (m_bOwned)
      EZ_DEFAULT_DELETE_ARRAY(m_pData);
  }

  SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() override
  {
    return m_pData.GetPtr();
  }

  SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override
  {
    return m_pData.GetCount();
  }

  ISlangUnknown* getInterface(const SlangUUID& uuid)
  {
    if (uuid == SLANG_UUID_ISlangBlob)
      return static_cast<ISlangBlob*>(this);

    if (uuid == SLANG_UUID_ISlangUnknown)
      return (ISlangUnknown*)this;

    return nullptr;
  }

  ezUInt32 addReference() const
  {
    return ezAtomicUtils::Increment(m_iRefCount);
  }

  ezUInt32 releaseReference()
  {
    auto const iRefCount = ezAtomicUtils::Decrement(m_iRefCount);

    if (iRefCount == 0)
    {
      if (m_bOwned)
      {
        EZ_DEFAULT_DELETE_ARRAY(m_pData);
        m_bOwned = false;
      }

      ezInternal::Delete(ezFoundation::GetDefaultAllocator(), this);
    }

    return iRefCount;
  }

  mutable ezInt32 m_iRefCount{0};
  ezByteArrayPtr m_pData;

  bool m_bOwned{false};
};

EZ_IMPLEMENT_SINGLETON(RPI::spShaderManager);

namespace RPI
{
  static ezStringView GetSlangType(ezEnum<RHI::spShaderSpecializationConstantType> eType)
  {
    switch (eType)
    {
      case RHI::spShaderSpecializationConstantType::Bool:
        return "bool"_ezsv;
      case RHI::spShaderSpecializationConstantType::UInt16:
        return "uint16_t"_ezsv;
      case RHI::spShaderSpecializationConstantType::Int16:
        return "int16_t"_ezsv;
      case RHI::spShaderSpecializationConstantType::UInt32:
        return "uint"_ezsv;
      default:
      case RHI::spShaderSpecializationConstantType::Int32:
        return "int"_ezsv;
      case RHI::spShaderSpecializationConstantType::UInt64:
        return "uint64_t"_ezsv;
      case RHI::spShaderSpecializationConstantType::Int64:
        return "int64_t"_ezsv;
      case RHI::spShaderSpecializationConstantType::Float:
        return "float"_ezsv;
      case RHI::spShaderSpecializationConstantType::Double:
        return "double"_ezsv;
    }
  }

  ezUInt32 spShaderCompilerSetup::CalculateHash() const
  {
    ezUInt32 uiHash = ezHashingUtils::StringHashTo32(m_hShaderResource.GetResourceIDHash());
    uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32(&m_eStage, sizeof(m_eStage)));

    for (const auto& macro : m_PredefinedMacros)
    {
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32String(macro.name));
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32String(macro.value));
    }

    for (const auto& constant : m_SpecializationConstants)
    {
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32String(constant.m_sName));
      RHI::spShaderSpecializationConstantType::Enum eType = constant.m_eType;
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32(&eType, sizeof(eType)));
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::xxHash32(&constant.m_uiValue, sizeof(constant.m_uiValue)));
    }

    if (m_hRootMaterialResource.IsValid())
    {
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashingUtils::StringHashTo32(m_hRootMaterialResource.GetResourceIDHash()));
    }

    return uiHash;
  }

  spShaderManager::spShaderManager()
    : m_SingletonRegistrar(this)
  {
    slang::createGlobalSession(m_pGlobalSession.writeRef());
  }

  spShaderManager::~spShaderManager()
  {
    for (auto& [uiHash, pShader] : m_ShaderKernelCache)
      pShader.Clear();

    m_ShaderKernelCache.Clear();
  }

  ezSharedPtr<RHI::spShader> spShaderManager::CompileShader(const spShaderCompilerSetup& compilerSetup)
  {
    if (!compilerSetup.m_hShaderResource.IsValid())
      return nullptr;

    const ezUInt32 uiHash = compilerSetup.CalculateHash();

    if (!m_ShaderKernelCache.Contains(uiHash))
    {
      ezDynamicArray<slang::PreprocessorMacroDesc> macros = compilerSetup.m_PredefinedMacros;

      if (compilerSetup.m_hRootMaterialResource.IsValid())
      {
        const ezResourceLock rootMaterialResource(compilerSetup.m_hRootMaterialResource, ezResourceAcquireMode::BlockTillLoaded);

        for (auto macro : rootMaterialResource->GetDescriptor().GetRootMaterial().GetMetadata().m_Macros)
          macros.PushBack({macro.Key().GetData(), macro.Value().GetData()});
      }

      auto const pSession = CreateSlangSession(macros.GetArrayPtr());
      LoadCoreModules(pSession);

      const ezResourceLock resource(compilerSetup.m_hShaderResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
      EZ_ASSERT_DEV(resource.IsValid(), "Unable to get the shader resource.");

      auto const pBytes = resource.GetPointer()->GetDescriptor().GetShader().GetShaderBytes();

      Slang::ComPtr<slang::IBlob> pDiagnostics;
      Slang::ComPtr<slang::IModule> pModule;
      std::vector<slang::IComponentType*> shaderProgramComponents;

      {
        const Slang::ComPtr<spSlangByteBlob> pBlob(EZ_DEFAULT_NEW(spSlangByteBlob, pBytes));

        ezStringBuilder sTemp;
        pModule = pSession->loadModuleFromSource("shader", resource->GetResourceID().GetData(sTemp), pBlob, pDiagnostics.writeRef());

        if (pDiagnostics)
        {
          ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));
          return nullptr;
        }

        shaderProgramComponents.push_back(pModule);
      }

      {
        const Slang::ComPtr<slang::IEntryPoint> pEntryPoint = GetEntryPointForStage(pModule, compilerSetup.m_eStage);

        if (pEntryPoint == nullptr)
        {
          ezLog::Error("Unable to find an entry point for stage {}", RHI::spShaderStage::ToString(compilerSetup.m_eStage));
          return nullptr;
        }

        shaderProgramComponents.push_back(pEntryPoint);
      }

      Slang::ComPtr<slang::IModule> pMaterialModule = nullptr;

      if (compilerSetup.m_hRootMaterialResource.IsValid())
      {
        pMaterialModule = CreateMaterialModule(pSession, compilerSetup);
      }

      const Slang::ComPtr<slang::IModule> pSpecializationModule = CreateSpecializationConstantsModule(pSession, compilerSetup);
      shaderProgramComponents.push_back(pSpecializationModule);

      Slang::ComPtr<slang::IComponentType> pShaderProgram;
      SlangResult result = pSession->createCompositeComponentType(shaderProgramComponents.data(), shaderProgramComponents.size(), pShaderProgram.writeRef(), pDiagnostics.writeRef());

      if (result != SLANG_OK || pDiagnostics)
      {
        ezLog::Error("Unable to create a shader program.");

        if (pDiagnostics)
          ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));

        return nullptr;
      }

      Slang::ComPtr<slang::IComponentType> pSpecializedProgram;

      // TODO: Either this or link-time specialization should be applied.
      // if (false)
      if (compilerSetup.m_hRootMaterialResource.IsValid() && pMaterialModule != nullptr && pShaderProgram->getSpecializationParamCount() > 0)
      {
        const ezResourceLock rootMaterialResource(compilerSetup.m_hRootMaterialResource, ezResourceAcquireMode::BlockTillLoaded);

        const auto materialType = pMaterialModule->getLayout()->findTypeByName(rootMaterialResource->GetDescriptor().GetRootMaterial().GetMetadata().m_sName);
        if (materialType == nullptr)
        {
          ezLog::Error("Unable to find the material type '{}'. Ensure the material type specified in the material descriptor is defined and implements the IMaterial interface.", rootMaterialResource->GetDescriptor().GetRootMaterial().GetMetadata().m_sName);
          return nullptr;
        }

        std::vector<slang::SpecializationArg> specializationArgs;

        {
          const slang::SpecializationArg effectTypeArg = slang::SpecializationArg::fromType(materialType);
          specializationArgs.push_back(effectTypeArg);
        }

        result = pShaderProgram->specialize(
          specializationArgs.data(),
          static_cast<SlangInt>(specializationArgs.size()),
          pSpecializedProgram.writeRef(),
          pDiagnostics.writeRef());

        if (result != SLANG_OK || pDiagnostics)
        {
          ezLog::Error("Unable to specialize the shader program.");

          if (pDiagnostics)
            ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));

          return nullptr;
        }
      }
      else
      {
        pSpecializedProgram = pShaderProgram;
      }

      Slang::ComPtr<slang::IComponentType> pLinkedShaderProgram;
      result = pSpecializedProgram->link(pLinkedShaderProgram.writeRef(), pDiagnostics.writeRef());

      if (result != SLANG_OK || pDiagnostics)
      {
        ezLog::Error("Unable to link the shader program.");

        if (pDiagnostics)
          ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));

        return nullptr;
      }

      Slang::ComPtr<slang::IBlob> pShaderKernel;
      result = pLinkedShaderProgram->getEntryPointCode(0, 0, pShaderKernel.writeRef(), pDiagnostics.writeRef());

      if (result != SLANG_OK || pDiagnostics)
      {
        ezLog::Error("Unable to compile the shader program.");

        if (pDiagnostics)
          ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));

        return nullptr;
      }

      slang::ProgramLayout* pShaderLayout = pShaderProgram->getLayout();
      const auto uiKernelSize = static_cast<ezUInt32>(pShaderKernel->getBufferSize());

      RHI::spShaderDescription desc;
      desc.m_eShaderStage = compilerSetup.m_eStage;
      desc.m_sEntryPoint.Assign(pShaderLayout->getEntryPointByIndex(0)->getName());
      desc.m_Buffer = EZ_DEFAULT_NEW_ARRAY(ezUInt8, uiKernelSize);
      ezMemoryUtils::RawByteCopy(desc.m_Buffer.GetPtr(), pShaderKernel->getBufferPointer(), uiKernelSize);
      // desc.m_Buffer[uiKernelSize] = '\0';
      desc.m_bOwnBuffer = true;

      const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<RHI::spDevice>();

      auto const pShader = pDevice->GetResourceFactory()->CreateShader(desc);

      CacheShaderKernel(uiHash, pShader);
    }

    return m_ShaderKernelCache[uiHash];
  }

  void spShaderManager::CompileRootMaterial(const spRootMaterialCompilerSetup& compilerSetup, Slang::ComPtr<slang::IBlob>& out_ir)
  {
    if (!ezFileSystem::ExistsFile(compilerSetup.m_sRootMaterialPath))
    {
      ezLog::Error("Root material file '{}' does not exist.", compilerSetup.m_sRootMaterialPath);
      return;
    }

    auto const pSession = CreateSlangSession(compilerSetup.m_PredefinedMacros.GetArrayPtr());

    ezFileReader fileReader;
    if (fileReader.Open(compilerSetup.m_sRootMaterialPath).Failed())
    {
      ezLog::Error("Failed to open root material file '{}'.", compilerSetup.m_sRootMaterialPath);
      return;
    }

    ezByteArrayPtr shaderCode = EZ_DEFAULT_NEW_ARRAY(ezUInt8, static_cast<ezUInt32>(fileReader.GetFileSize()));
    EZ_SCOPE_EXIT(EZ_DEFAULT_DELETE_ARRAY(shaderCode));

    fileReader.ReadBytes(shaderCode.GetPtr(), shaderCode.GetCount());

    const Slang::ComPtr<spSlangByteBlob> pCode(EZ_DEFAULT_NEW(spSlangByteBlob, shaderCode));

    ezStringBuilder sTemp;
    Slang::ComPtr<slang::IModule> pModule(pSession->loadModuleFromSource("material", compilerSetup.m_sRootMaterialPath.GetData(sTemp), pCode));

    if (const SlangResult result = pModule->serialize(out_ir.writeRef()); result != SLANG_OK)
      ezLog::Error("Failed to serialize root material module.");
  }

  void spShaderManager::CacheShaderKernel(ezUInt32 uiHash, ezSharedPtr<RHI::spShader> pShaderKernel)
  {
    m_ShaderKernelCache.Insert(uiHash, pShaderKernel);
  }

  Slang::ComPtr<slang::IModule> spShaderManager::CreateSpecializationConstantsModule(Slang::ComPtr<slang::ISession> pSession, const spShaderCompilerSetup& compilerSetup)
  {
    ezStringBuilder shaderCode;

    ezDynamicArray<RHI::spShaderSpecializationConstant> specializationConstants = compilerSetup.m_SpecializationConstants;

    // TODO: Apply specialization constants from the material instance.
    // if (compilerSetup.m_hRootMaterialResource.IsValid())
    // {
    //   const ezResourceLock materialResource(compilerSetup.m_hRootMaterialResource, ezResourceAcquireMode::BlockTillLoaded);
    //
    //   const auto& constants = materialResource->GetDescriptor().GetRootMaterial().GetMetadata().m_SpecializationConstants;
    //   specializationConstants.PushBackRange(constants.GetArrayPtr());
    // }

    for (const auto& specialization : compilerSetup.m_SpecializationConstants)
    {
      shaderCode.AppendFormat("export static const {} o_{} = ", GetSlangType(specialization.m_eType), specialization.m_sName.GetData());

      switch (specialization.m_eType)
      {
        case RHI::spShaderSpecializationConstantType::Bool:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<bool>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::Float:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<float>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::Double:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<double>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::Int16:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<ezInt16>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::Int32:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<ezInt32>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::Int64:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<ezInt64>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::UInt16:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<ezUInt16>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::UInt32:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<ezUInt32>(specialization.m_uiValue));
          break;
        case RHI::spShaderSpecializationConstantType::UInt64:
          shaderCode.AppendFormat("{};", RHI::spShaderSpecializationConstant::Get<ezUInt64>(specialization.m_uiValue));
          break;
        default:
          shaderCode.AppendFormat("0;");
          break;
      }

      shaderCode.Append("\n");
    }

    const Slang::ComPtr<spSlangByteBlob> pCode(EZ_DEFAULT_NEW(spSlangByteBlob, shaderCode));
    ezLog::Info("{}", shaderCode);

    Slang::ComPtr<slang::IBlob> pDiagnostics;
    Slang::ComPtr<slang::IModule> pModule(pSession->loadModuleFromSource("specialization_constants", "specialization-constants.slang", pCode, pDiagnostics.writeRef()));

    if (pDiagnostics != nullptr)
    {
      ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));
      return nullptr;
    }

    return pModule;
  }

  Slang::ComPtr<slang::IModule> spShaderManager::CreateMaterialModule(Slang::ComPtr<slang::ISession> pSession, const spShaderCompilerSetup& compilerSetup)
  {
    const ezResourceLock rootMaterialResource(compilerSetup.m_hRootMaterialResource, ezResourceAcquireMode::BlockTillLoaded);

    const Slang::ComPtr<spSlangByteBlob> pIR(EZ_DEFAULT_NEW(spSlangByteBlob, rootMaterialResource->GetDescriptor().GetRootMaterial().GetShaderBytes()));

    Slang::ComPtr<slang::IBlob> pDiagnostics;
    Slang::ComPtr<slang::IModule> pModule(pSession->loadModuleFromSource("material", "material.slang", pIR));

    if (pDiagnostics != nullptr)
    {
      ezLog::Error("{}", static_cast<const char*>(pDiagnostics->getBufferPointer()));
      return nullptr;
    }

    return pModule;
  }

  Slang::ComPtr<slang::IEntryPoint> spShaderManager::GetEntryPointForStage(Slang::ComPtr<slang::IModule> pModule, ezEnum<RHI::spShaderStage> eStage)
  {
    const ezInt32 uiEntryPointCount = pModule->getDefinedEntryPointCount();

    Slang::ComPtr<slang::IEntryPoint> pEntryPoint;

    for (ezInt32 i = 0; i < uiEntryPointCount; ++i)
    {
      pModule->getDefinedEntryPoint(i, pEntryPoint.writeRef());
      slang::ProgramLayout* pReflection = pEntryPoint->getLayout();

      auto* pEntryPointReflection = pReflection->getEntryPointByIndex(0);
      const SlangStage eSlangStage = pEntryPointReflection->getStage();

      if (
        (eStage == RHI::spShaderStage::VertexShader && eSlangStage == SLANG_STAGE_VERTEX) ||
        (eStage == RHI::spShaderStage::PixelShader && eSlangStage == SLANG_STAGE_PIXEL) ||
        (eStage == RHI::spShaderStage::DomainShader && eSlangStage == SLANG_STAGE_DOMAIN) ||
        (eStage == RHI::spShaderStage::HullShader && eSlangStage == SLANG_STAGE_HULL) ||
        (eStage == RHI::spShaderStage::GeometryShader && eSlangStage == SLANG_STAGE_GEOMETRY) ||
        (eStage == RHI::spShaderStage::ComputeShader && eSlangStage == SLANG_STAGE_COMPUTE) ||
        (eStage == RHI::spShaderStage::None && eSlangStage == SLANG_STAGE_NONE))
      {
        break;
      }
    }

    return pEntryPoint;
  }

  void spShaderManager::LoadCoreModules(Slang::ComPtr<slang::ISession> pSession)
  {
    ezStringBuilder sTemp;

    for (auto module : {"Common", "Math", "MaterialSystem"})
    {
      ezFileReader file;
      if (file.Open(ezFmt(":shaders/Lib/Private/{}.slang", module).GetText(sTemp)).Failed())
        return;

      const ezStringBuilder sAbsolutePath = file.GetFilePathAbsolute();

      ezByteArrayPtr shaderCode = EZ_DEFAULT_NEW_ARRAY(ezUInt8, static_cast<ezUInt32>(file.GetFileSize()));
      EZ_SCOPE_EXIT(EZ_DEFAULT_DELETE_ARRAY(shaderCode));

      file.ReadBytes(shaderCode.GetPtr(), shaderCode.GetCount());

      const Slang::ComPtr<spSlangByteBlob> pCode(EZ_DEFAULT_NEW(spSlangByteBlob, shaderCode));

      pSession->loadModuleFromSource(module, sAbsolutePath.GetData(), pCode);
    }
  }

  Slang::ComPtr<slang::ISession> spShaderManager::CreateSlangSession(const ezArrayPtr<const slang::PreprocessorMacroDesc>& predefinedMacros) const
  {
    slang::SessionDesc desc;
    const auto* pDevice = ezSingletonRegistry::GetRequiredSingletonInstance<RHI::spDevice>();

    slang::TargetDesc targetDesc;
    const ezEnum<RHI::spGraphicsApi> eGraphicsApi = pDevice->GetAPI();

    if (eGraphicsApi == RHI::spGraphicsApi::Direct3D11)
    {
      targetDesc.format = SLANG_DXBC;
      targetDesc.profile = m_pGlobalSession->findProfile("sm_5_0");
    }
    else if (eGraphicsApi == RHI::spGraphicsApi::Direct3D12)
    {
      targetDesc.format = SLANG_DXIL;
      targetDesc.profile = m_pGlobalSession->findProfile("sm_6_6");
    }
    else if (eGraphicsApi == RHI::spGraphicsApi::Vulkan)
    {
      targetDesc.format = SLANG_SPIRV;
      targetDesc.profile = m_pGlobalSession->findProfile("glsl_450");
    }
    else if (eGraphicsApi == RHI::spGraphicsApi::Metal)
    {
      targetDesc.format = SLANG_METAL;
      targetDesc.profile = m_pGlobalSession->findProfile("metallib_2_3");
    }
    else if (eGraphicsApi == RHI::spGraphicsApi::OpenGL)
    {
      targetDesc.format = SLANG_GLSL;
      targetDesc.profile = m_pGlobalSession->findProfile("glsl_450");
    }
    else if (eGraphicsApi == RHI::spGraphicsApi::OpenGLES)
    {
      targetDesc.format = SLANG_GLSL;
      targetDesc.profile = m_pGlobalSession->findProfile("glsl_330");
    }

    desc.targets = &targetDesc;
    desc.targetCount = 1;

    desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    ezStringBuilder sEnginePrivateShadersPathBuilder(ezFileSystem::GetSdkRootDirectory());
    sEnginePrivateShadersPathBuilder.AppendPath("Shaders", "Lib", "Private");

    ezStringBuilder sEnginePublicShadersPathBuilder(ezFileSystem::GetSdkRootDirectory());
    sEnginePublicShadersPathBuilder.AppendPath("Shaders", "Lib", "Public");

    ezStringBuilder sProjectPrivateShadersPathBuilder;
    ezStringBuilder sProjectPublicShadersPathBuilder;

    ezDynamicArray<const char*> searchPaths;
    searchPaths.PushBack(sEnginePrivateShadersPathBuilder);
    searchPaths.PushBack(sEnginePublicShadersPathBuilder);

    if (ezFileSystem::ResolvePath(":project/Shaders/Lib/Private", &sProjectPrivateShadersPathBuilder, nullptr).Succeeded())
      searchPaths.PushBack(sProjectPrivateShadersPathBuilder);
    if (ezFileSystem::ResolvePath(":project/Shaders/Lib/Public", &sProjectPublicShadersPathBuilder, nullptr).Succeeded())
      searchPaths.PushBack(sProjectPublicShadersPathBuilder);

    desc.searchPaths = searchPaths.GetData();
    desc.searchPathCount = searchPaths.GetCount();

    ezDynamicArray<slang::PreprocessorMacroDesc> sessionMacros;
    sessionMacros.PushBackRange(predefinedMacros);

    ezStringBuilder sbTempStorage;
    sessionMacros.PushBack({"SP_RHI_API", ezFmt("{}", eGraphicsApi.GetValue()).GetTextCStr(sbTempStorage)});
    sessionMacros.PushBack({"SP_RHI_SHADER", "1"});

    desc.preprocessorMacros = sessionMacros.GetData();
    desc.preprocessorMacroCount = sessionMacros.GetCount();

    Slang::ComPtr<slang::ISession> session;

    if (const SlangResult res = m_pGlobalSession->createSession(desc, session.writeRef()); res == SLANG_OK)
      return session;

    return nullptr;
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Shaders_ShaderManager)
