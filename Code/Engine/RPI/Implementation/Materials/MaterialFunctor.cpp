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

#include <RPI/RPIPCH.h>

#include <RPI/Materials/MaterialFunctor.h>

#include <RPI/Materials/Functors/MaterialFunctor_init.h>
#include <RPI/Materials/Functors/MaterialFunctor_isset.h>
#include <RPI/Materials/Functors/MaterialFunctor_ref.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(RPI::spMaterialFunctorEvaluator, ezNoBase, 1, ezRTTIDefaultAllocator<RPI::spMaterialFunctorEvaluator>)
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMaterialFunctor, 1, ezRTTINoAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_IMPLEMENT_SINGLETON(spMaterialFuntorRegistry);

  EZ_DEFINE_CUSTOM_VARIANT_TYPE(spMaterialFunctorEvaluator);
  // clang-format on

  spMaterialFunctor::spMaterialFunctor(ezStringView sName)
  {
    m_sName.Assign(sName);
  }

  spMaterialFuntorRegistry::spMaterialFuntorRegistry()
    : m_SingletonRegistrar(this)
  {
    m_Functors.Clear();
  }

  spMaterialFuntorRegistry::~spMaterialFuntorRegistry()
  {
    for (auto& pFunctor : m_Functors)
    {
      EZ_DEFAULT_DELETE(pFunctor);
      pFunctor = nullptr;
    }

    m_Functors.Clear();
  }

  void spMaterialFuntorRegistry::Register(spMaterialFunctor* pMaterialFunctor)
  {
    m_Functors.PushBack(pMaterialFunctor);
  }

  void spMaterialFuntorRegistry::Unregister(spMaterialFunctor* pMaterialFunctor)
  {
    m_Functors.RemoveAndSwap(pMaterialFunctor);
  }

  const spMaterialFunctor* spMaterialFuntorRegistry::Get(ezTempHashedString sName)
  {
    if (s_pSingleton == nullptr)
      return nullptr;

    for (const auto& pFunctor : s_pSingleton->m_Functors)
      if (pFunctor->GetName() == sName)
        return pFunctor;

    return nullptr;
  }

  bool spMaterialFuntorRegistry::Contains(ezTempHashedString sName)
  {
    if (s_pSingleton == nullptr)
      return false;

    for (const auto& pFunctor : s_pSingleton->m_Functors)
      if (pFunctor->GetName() == sName)
        return true;

    return false;
  }
} // namespace RPI

void operator<<(ezStreamWriter& inout_stream, const RPI::spMaterialFunctorEvaluator& value)
{
  inout_stream.WriteBytes(&value.m_pFunctor, sizeof(RPI::spMaterialFunctor*)).IgnoreResult();
  inout_stream.WriteArray(value.m_Arguments).IgnoreResult();
}

void operator>>(ezStreamReader& inout_stream, RPI::spMaterialFunctorEvaluator& out_value)
{
  inout_stream.ReadBytes(&out_value.m_pFunctor, sizeof(RPI::spMaterialFunctor*));
  inout_stream.ReadArray(out_value.m_Arguments).IgnoreResult();
}

static RPI::spMaterialFuntorRegistry* s_pFunctorRegistry = nullptr;

EZ_BEGIN_SUBSYSTEM_DECLARATION(RPI, MaterialFunctor)

  // clang-format off
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core",
    "RAI"
  END_SUBSYSTEM_DEPENDENCIES
  // clang-format on

  ON_BASESYSTEMS_STARTUP
  {
    s_pFunctorRegistry = EZ_DEFAULT_NEW(RPI::spMaterialFuntorRegistry);
  }

  ON_CORESYSTEMS_STARTUP
  {
    s_pFunctorRegistry->Register(EZ_DEFAULT_NEW(RPI::spMaterialFunctor_init));
    s_pFunctorRegistry->Register(EZ_DEFAULT_NEW(RPI::spMaterialFunctor_isset));
    s_pFunctorRegistry->Register(EZ_DEFAULT_NEW(RPI::spMaterialFunctor_ref));
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    EZ_DEFAULT_DELETE(s_pFunctorRegistry);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION;
