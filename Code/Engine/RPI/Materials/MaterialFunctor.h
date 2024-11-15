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

SP_DECLARE_SUBSYSTEM(RPI, MaterialFunctor);

namespace RPI
{
  class SP_RPI_DLL spMaterialFunctor : public ezReflectedClass
  {
    EZ_ADD_DYNAMIC_REFLECTION(spMaterialFunctor, ezReflectedClass);

  public:
    explicit spMaterialFunctor(ezStringView sName);
    ~spMaterialFunctor() override = default;

    virtual ezVariant Evaluate(const ezArrayPtr<ezVariant>& arguments) const = 0;

    [[nodiscard]] EZ_ALWAYS_INLINE const ezHashedString& GetName() const { return m_sName; }

  private:
    ezHashedString m_sName;
  };

  struct spMaterialFunctorEvaluator
  {
    const spMaterialFunctor* m_pFunctor{nullptr};
    ezDynamicArray<ezVariant> m_Arguments;

    [[nodiscard]] EZ_FORCE_INLINE ezVariant operator()(const ezArrayPtr<ezVariant>& arguments)
    {
      return m_pFunctor->Evaluate(m_Arguments.GetArrayPtr());
    }
  };

  class SP_RPI_DLL spMaterialFuntorRegistry
  {
    EZ_MAKE_SUBSYSTEM_STARTUP_FRIEND(RPI, MaterialFunctor);
    EZ_DECLARE_SINGLETON(spMaterialFuntorRegistry);

  public:
    spMaterialFuntorRegistry();
    ~spMaterialFuntorRegistry();

    void Register(spMaterialFunctor* pMaterialFunctor);
    void Unregister(spMaterialFunctor* pMaterialFunctor);

    [[nodiscard]] static const spMaterialFunctor* Get(ezTempHashedString sName);

    [[nodiscard]] static bool Contains(ezTempHashedString sName);

  private:
    ezHybridArray<spMaterialFunctor*, 16> m_Functors;
  };
} // namespace RPI

template <>
struct ezHashHelper<RPI::spMaterialFunctorEvaluator>
{
  EZ_ALWAYS_INLINE static ezUInt32 Hash(const RPI::spMaterialFunctorEvaluator& value)
  {
    ezUInt32 uiHash = 0;
    for (const auto& argument : value.m_Arguments)
      uiHash = ezHashingUtils::CombineHashValues32(uiHash, ezHashHelper<ezVariant>::Hash(argument));

    return ezHashingUtils::CombineHashValues32(ezHashingUtils::xxHash32(&value.m_pFunctor, sizeof(RPI::spMaterialFunctor*)), uiHash);
  }

  EZ_ALWAYS_INLINE static bool Equal(const RPI::spMaterialFunctorEvaluator& a, const RPI::spMaterialFunctorEvaluator& b)
  {
    return Hash(a) == Hash(b);
  }
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RPI_DLL, RPI::spMaterialFunctorEvaluator);
EZ_DECLARE_CUSTOM_VARIANT_TYPE(RPI::spMaterialFunctorEvaluator);

SP_RPI_DLL void operator<<(ezStreamWriter& inout_stream, const RPI::spMaterialFunctorEvaluator& value);
SP_RPI_DLL void operator>>(ezStreamReader& inout_stream, RPI::spMaterialFunctorEvaluator& out_value);
