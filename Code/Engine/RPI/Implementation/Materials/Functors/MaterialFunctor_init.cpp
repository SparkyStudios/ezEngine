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

#include <RPI/Materials/Functors/MaterialFunctor_init.h>

namespace RPI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMaterialFunctor_init, 1, ezRTTIDefaultAllocator<spMaterialFunctor_init>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;
  // clang-format on

  static ezHashTable<ezHashedString, const ezRTTI*> InitializePropertyValueTypes()
  {
    ezHashTable<ezHashedString, const ezRTTI*> types;
    types.Insert(ezMakeHashedString("bool"), ezGetStaticRTTI<bool>());

    types.Insert(ezMakeHashedString("float"), ezGetStaticRTTI<float>());
    types.Insert(ezMakeHashedString("float2"), ezGetStaticRTTI<ezVec2>());
    types.Insert(ezMakeHashedString("float3"), ezGetStaticRTTI<ezVec3>());
    types.Insert(ezMakeHashedString("float4"), ezGetStaticRTTI<ezVec4>());

    types.Insert(ezMakeHashedString("int"), ezGetStaticRTTI<ezInt32>());
    types.Insert(ezMakeHashedString("int2"), ezGetStaticRTTI<ezVec2I32>());
    types.Insert(ezMakeHashedString("int3"), ezGetStaticRTTI<ezVec3I32>());
    types.Insert(ezMakeHashedString("int4"), ezGetStaticRTTI<ezVec4I32>());

    types.Insert(ezMakeHashedString("uint"), ezGetStaticRTTI<ezUInt32>());
    types.Insert(ezMakeHashedString("uint2"), ezGetStaticRTTI<ezVec2U32>());
    types.Insert(ezMakeHashedString("uint3"), ezGetStaticRTTI<ezVec3U32>());
    types.Insert(ezMakeHashedString("uint4"), ezGetStaticRTTI<ezVec4U32>());

    types.Insert(ezMakeHashedString("Color"), ezGetStaticRTTI<ezColor>());

    types.Insert(ezMakeHashedString("Texture2D"), ezGetStaticRTTI<ezString>());
    // TODO: More texture types

    return types;
  }

  static spLazy<ezHashTable<ezHashedString, const ezRTTI*>> s_MaterialPropertyValueTypes{InitializePropertyValueTypes};

  static const ezRTTI* GetMaterialPropertyValueType(ezTempHashedString sTypeName)
  {
    InitializePropertyValueTypes();

    const ezRTTI* pType = nullptr;
    s_MaterialPropertyValueTypes.Get().TryGetValue(sTypeName, pType);
    return pType;
  }

  spMaterialFunctor_init::spMaterialFunctor_init()
    : spMaterialFunctor("init")
  {
  }

  ezVariant spMaterialFunctor_init::Evaluate(const ezArrayPtr<ezVariant>& arguments) const
  {
    if (arguments.GetCount() == 0)
      return ezVariant();

    ezResult conversionResult = EZ_FAILURE;
    const auto sTypeName = arguments[0].ConvertTo<ezString>(&conversionResult);

    const ezRTTI* pType = GetMaterialPropertyValueType(ezTempHashedString(sTypeName));

    if (conversionResult.Failed())
      return ezVariant();

    auto constructorArgs = arguments.GetSubArray(1, arguments.GetCount() - 1);

    auto functions = pType->GetFunctions();
    for (const auto& pFunc : functions)
    {
      if (pFunc->GetFunctionType() != ezFunctionType::Constructor || pFunc->GetArgumentCount() != constructorArgs.GetCount())
        continue;

      ezHybridArray<ezVariant, 8> convertedArgs;
      bool bAllArgsValid = true;

      for (ezUInt32 uiArg = 0; uiArg < pFunc->GetArgumentCount(); ++uiArg)
      {
        const ezRTTI* pArgType = pFunc->GetArgumentType(uiArg);
        convertedArgs.PushBack(constructorArgs[uiArg].ConvertTo(pArgType->GetVariantType(), &conversionResult));

        if (conversionResult.Succeeded())
          continue;

        bAllArgsValid = false;
        break;
      }

      if (!bAllArgsValid)
        continue;

      ezVariant result;
      pFunc->Execute(nullptr, convertedArgs, result);

      if (result.IsValid())
        return result;
    }

    return ezVariant();
  }
} // namespace RPI
