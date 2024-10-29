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

#include <RPI/Materials/MaterialParser.h>

#include <Foundation/CodeUtils/TokenParseUtils.h>
#include <Foundation/IO/FileSystem/FileReader.h>

#include <MaterialFlags.h>

using namespace ezTokenParseUtils;

namespace RPI
{
  static ezVariant ParseValue(const TokenStream& tokens, ezUInt32& ref_uiTokenIndex)
  {
    ezUInt32 uiTokenIndex = ref_uiTokenIndex;

    if (Accept(tokens, ref_uiTokenIndex, ezTokenType::String1, &uiTokenIndex) || Accept(tokens, ref_uiTokenIndex, ezTokenType::String2, &uiTokenIndex))
    {
      ezStringBuilder sValue = tokens[uiTokenIndex]->m_DataView;
      sValue.Trim("\"'");

      return ezVariant(sValue.GetData());
    }

    if (Accept(tokens, ref_uiTokenIndex, ezTokenType::Integer, &uiTokenIndex))
    {
      const ezString sValue = tokens[uiTokenIndex]->m_DataView;
      ezInt64 iValue64 = 0;

      if (sValue.StartsWith_NoCase("0x"))
      {
        ezUInt32 uiValue32 = 0;
        ezConversionUtils::ConvertHexStringToUInt32(sValue, uiValue32).IgnoreResult();

        iValue64 = uiValue32;
      }
      else
      {
        ezConversionUtils::StringToInt64(sValue, iValue64).IgnoreResult();
      }

      return ezVariant(iValue64);
    }

    if (Accept(tokens, ref_uiTokenIndex, ezTokenType::Float, &uiTokenIndex))
    {
      const ezString sValue = tokens[uiTokenIndex]->m_DataView;

      double dValue = 0.0;
      ezConversionUtils::StringToFloat(sValue, dValue).IgnoreResult();

      return ezVariant(dValue);
    }

    if (Accept(tokens, ref_uiTokenIndex, "true", &uiTokenIndex) || Accept(tokens, ref_uiTokenIndex, "false", &uiTokenIndex))
    {
      const bool bValue = tokens[uiTokenIndex]->m_DataView == "true";
      return ezVariant(bValue);
    }

    if (Accept(tokens, ref_uiTokenIndex, "@"))
    {
      const spMaterialFunctor* pRefFunctor = spMaterialFuntorRegistry::Get("ref");
      if (pRefFunctor == nullptr)
      {
        ezLog::Error("Cannot parse the material. Unable to find the builtin 'ref' functor.");
        return ezVariant();
      }

      if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Identifier, &uiTokenIndex))
      {
        ezLog::Error("Cannot parse the material. Expected a property name after '@'.");
        return ezVariant();
      }

      const ezString sPropertyName = tokens[uiTokenIndex]->m_DataView;

      ezDynamicArray<ezVariant> arguments;
      arguments.PushBack(ezVariant(sPropertyName));

      return ezVariant(spMaterialFunctorEvaluator{pRefFunctor, arguments});
    }

    if (auto& sDataView = tokens[ref_uiTokenIndex]->m_DataView; tokens[ref_uiTokenIndex]->m_iType == ezTokenType::Identifier && ezStringUtils::IsValidIdentifierName(sDataView.GetStartPointer(), sDataView.GetEndPointer()))
    {
      const ezTempHashedString sDataHash(sDataView);
      const spMaterialFunctor* pFunctor = spMaterialFuntorRegistry::Get(sDataHash);

      ezDynamicArray<ezVariant> arguments;

      if (pFunctor == nullptr)
      {
        pFunctor = spMaterialFuntorRegistry::Get("init");

        if (pFunctor == nullptr)
        {
          ezLog::Error("Cannot parse the material. The identifier '{0}' in not a functor.", sDataView);
          return ezVariant();
        }

        arguments.PushBack(ezVariant(sDataView));
      }

      ++ref_uiTokenIndex;
      if (!Accept(tokens, ref_uiTokenIndex, "("))
      {
        ezLog::Error("Cannot parse the material. Invalid functor syntax for '{0}'.", sDataView);
        return ezVariant();
      }

      while (!Accept(tokens, ref_uiTokenIndex, ")"))
      {
        if (const ezVariant value = ParseValue(tokens, ref_uiTokenIndex); value.IsValid())
        {
          arguments.PushBack(value);
        }
        else
        {
          ezLog::Error("Invalid arguments for functor '{0}'.", pFunctor->GetName());
          return ezVariant();
        }

        Accept(tokens, ref_uiTokenIndex, ",");
      }

      return ezVariant(spMaterialFunctorEvaluator{pFunctor, arguments});
    }

    return ezVariant();
  }

  static ezResult ParseFlagMember(const TokenStream& tokens, ezUInt32& ref_uiTokenIndex, ezMap<ezUInt32, ezVariant>& ref_Flags)
  {
    ezUInt32 uiTokenIndex = ref_uiTokenIndex;
    if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Identifier, &uiTokenIndex))
      return EZ_FAILURE;

    const ezString sFlagName = tokens[uiTokenIndex]->m_DataView;

    ezUInt32 uiFlag = 0;
    if (sFlagName.IsEqual_NoCase("HasTextureAlbedo"))
      uiFlag = k_HasTextureAlbedoMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureNormal"))
      uiFlag = k_HasTextureNormalMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureMetalness"))
      uiFlag = k_HasTextureMetalnessMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureRoughness"))
      uiFlag = k_HasTextureRoughnessMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureOcclusion"))
      uiFlag = k_HasTextureOcclusionMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureCavity"))
      uiFlag = k_HasTextureCavityMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureORMC"))
      uiFlag = k_HasTextureORMCMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureSpecular"))
      uiFlag = k_HasTextureSpecularMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureEmissive"))
      uiFlag = k_HasTextureEmissiveMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureAlpha"))
      uiFlag = k_HasTextureAlphaMask;
    else if (sFlagName.IsEqual_NoCase("HasTextureHeight"))
      uiFlag = k_HasTextureHeightMask;
    else if (sFlagName.IsEqual_NoCase("CustomFlag"))
    {
      if (!Accept(tokens, ref_uiTokenIndex, "["))
        return EZ_FAILURE;

      ezUInt32 uiCustomFlag = 0;

      uiTokenIndex = ref_uiTokenIndex;
      if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Integer, &uiTokenIndex) || ezConversionUtils::StringToUInt(tokens[uiTokenIndex]->m_DataView, uiCustomFlag).Failed())
        return EZ_FAILURE;

      uiFlag = k_MaxReservedMateriaMask + uiCustomFlag;

      if (!Accept(tokens, ref_uiTokenIndex, "]"))
        return EZ_FAILURE;
    }

    if (!Accept(tokens, ref_uiTokenIndex, "="))
      return EZ_FAILURE;

    const ezVariant value = ParseValue(tokens, ref_uiTokenIndex);
    if (!value.IsValid())
      return EZ_FAILURE;

    if (!Accept(tokens, ref_uiTokenIndex, ";"))
      return EZ_FAILURE;

    ref_Flags[uiFlag] = value;
    return EZ_SUCCESS;
  }

  static ezResult ParseDataMember(const TokenStream& tokens, ezUInt32& ref_uiTokenIndex, ezMap<ezHashedString, ezVariant>& ref_Data)
  {
    ezUInt32 uiTokenIndex = ref_uiTokenIndex;
    if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Identifier, &uiTokenIndex))
      return EZ_FAILURE;

    ezHashedString sDataPropertyName;
    sDataPropertyName.Assign(tokens[uiTokenIndex]->m_DataView);

    if (!Accept(tokens, ref_uiTokenIndex, "="))
      return EZ_FAILURE;

    const ezVariant value = ParseValue(tokens, ref_uiTokenIndex);
    if (!value.IsValid())
      return EZ_FAILURE;

    if (!Accept(tokens, ref_uiTokenIndex, ";"))
      return EZ_FAILURE;

    ref_Data[sDataPropertyName] = value;
    return EZ_SUCCESS;
  }

  static ezResult ParseSpecializationConstant(const TokenStream& tokens, ezUInt32& ref_uiTokenIndex, ezMap<ezHashedString, ezVariant>& ref_Constants)
  {
    return ParseDataMember(tokens, ref_uiTokenIndex, ref_Constants);
  }

  static ezResult ParseAttribute(const TokenStream& tokens, ezUInt32& ref_uiTokenIndex, spMaterialPropertyAttributeDefinition& out_Attribute)
  {
    if (!Accept(tokens, ref_uiTokenIndex, "["))
      return EZ_FAILURE;

    ezUInt32 uiNameIndex = ref_uiTokenIndex;
    if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Identifier, &uiNameIndex))
      return EZ_FAILURE;

    out_Attribute.m_sName.Assign(tokens[uiNameIndex]->m_DataView);

    if (Accept(tokens, ref_uiTokenIndex, "("))
    {
      while (!Accept(tokens, ref_uiTokenIndex, ")"))
      {
        if (const ezVariant value = ParseValue(tokens, ref_uiTokenIndex); value.IsValid())
        {
          out_Attribute.m_Arguments.PushBack(value);
        }
        else
        {
          ezLog::Error("Invalid arguments for attribute '{0}'.", out_Attribute.m_sName);
          return EZ_FAILURE;
        }

        Accept(tokens, ref_uiTokenIndex, ",");
      }
    }

    if (!Accept(tokens, ref_uiTokenIndex, "]"))
      return EZ_FAILURE;

    return EZ_SUCCESS;
  }

  static ezResult ParseProperty(const TokenStream& tokens, ezUInt32& ref_uiTokenIndex, spMaterialPropertyDefinition& out_Property)
  {
    while (true)
    {
      spMaterialPropertyAttributeDefinition attribute;
      if (ParseAttribute(tokens, ref_uiTokenIndex, attribute).Failed())
        break;

      out_Property.m_Attributes.PushBack(std::move(attribute));
    }

    ezUInt32 uiTypeIndex = ref_uiTokenIndex;
    if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Identifier, &uiTypeIndex))
      return EZ_FAILURE;

    ezUInt32 uiNameIndex = ref_uiTokenIndex;
    if (!Accept(tokens, ref_uiTokenIndex, ezTokenType::Identifier, &uiNameIndex))
      return EZ_FAILURE;

    if (Accept(tokens, ref_uiTokenIndex, "="))
    {
      const ezVariant value = ParseValue(tokens, ref_uiTokenIndex);
      if (!value.IsValid())
        return EZ_FAILURE;

      out_Property.m_Initializer = value;
    }

    if (!Accept(tokens, ref_uiTokenIndex, ";"))
      return EZ_FAILURE;

    out_Property.m_sType = tokens[uiTypeIndex]->m_DataView;
    out_Property.m_sName.Assign(tokens[uiNameIndex]->m_DataView);

    return EZ_SUCCESS;
  }

  ezResult spMaterialParser::ParseMaterialMetadata(ezStringView sMaterialFilePath, spMaterialMetadata& out_MaterialMetadata)
  {
    ezFileReader file;
    if (file.Open(sMaterialFilePath).Failed())
      return EZ_FAILURE;

    ezByteArrayPtr pBytes = EZ_DEFAULT_NEW_ARRAY(ezUInt8, file.GetFileSize());
    EZ_SCOPE_EXIT(EZ_DEFAULT_DELETE_ARRAY(pBytes));

    file.ReadBytes(pBytes.GetPtr(), file.GetFileSize());

    ezTokenizer tokenizer;
    tokenizer.Tokenize(pBytes, ezLog::GetThreadLocalLogSystem(), false);

    TokenStream tokens;
    tokenizer.GetAllLines(tokens);

    ezUInt32 uiCurToken = 0;
    SkipUntil(tokens, uiCurToken, ezTokenType::LineComment, "// BEGIN_MATERIAL_DESCRIPTOR");

    if (!Accept(tokens, uiCurToken, "__ignored_block"))
      return EZ_FAILURE;

    if (!Accept(tokens, uiCurToken, "{"))
      return EZ_FAILURE;

    while (!Accept(tokens, uiCurToken, "}"))
    {
      ezUInt32 uiTokenIndex = uiCurToken;
      if (!Accept(tokens, uiCurToken, ezTokenType::Identifier, &uiTokenIndex))
        return EZ_FAILURE;

      if (const ezString sToken = tokens[uiTokenIndex]->m_DataView; sToken.IsEqual_NoCase("Name"))
      {
        if (!Accept(tokens, uiCurToken, "="))
          return EZ_FAILURE;

        if (!Accept(tokens, uiCurToken, ezTokenType::Identifier, &uiTokenIndex))
          return EZ_FAILURE;

        out_MaterialMetadata.m_sName.Assign(tokens[uiTokenIndex]->m_DataView);

        if (!Accept(tokens, uiCurToken, ";"))
          return EZ_FAILURE;
      }
      else if (sToken.IsEqual_NoCase("Description"))
      {
        if (!Accept(tokens, uiCurToken, "="))
          return EZ_FAILURE;

        if (!Accept(tokens, uiCurToken, ezTokenType::String1, &uiTokenIndex))
          return EZ_FAILURE;

        out_MaterialMetadata.m_sDescription = tokens[uiTokenIndex]->m_DataView;

        if (!Accept(tokens, uiCurToken, ";"))
          return EZ_FAILURE;
      }
      else if (sToken.IsEqual_NoCase("Flags"))
      {
        if (!Accept(tokens, uiCurToken, "{"))
          return EZ_FAILURE;

        while (!Accept(tokens, uiCurToken, "}"))
          if (ParseFlagMember(tokens, uiCurToken, out_MaterialMetadata.m_Flags).Failed())
            return EZ_FAILURE;
      }
      else if (sToken.IsEqual_NoCase("Data"))
      {
        if (!Accept(tokens, uiCurToken, "{"))
          return EZ_FAILURE;

        while (!Accept(tokens, uiCurToken, "}"))
          if (ParseDataMember(tokens, uiCurToken, out_MaterialMetadata.m_Data).Failed())
            return EZ_FAILURE;
      }
      else if (sToken.IsEqual_NoCase("Properties"))
      {
        if (!Accept(tokens, uiCurToken, "{"))
          return EZ_FAILURE;

        while (!Accept(tokens, uiCurToken, "}"))
        {
          spMaterialPropertyDefinition property;
          if (ParseProperty(tokens, uiCurToken, property).Failed())
            return EZ_FAILURE;

          ezHashedString sPropertyName;
          sPropertyName.Assign(property.m_sName);
          out_MaterialMetadata.m_Properties[sPropertyName] = std::move(property);
        }
      }
      else if (sToken.IsEqual_NoCase("SpecializationConstants"))
      {
        if (!Accept(tokens, uiCurToken, "{"))
          return EZ_FAILURE;

        while (!Accept(tokens, uiCurToken, "}"))
          if (ParseSpecializationConstant(tokens, uiCurToken, out_MaterialMetadata.m_SpecializationConstants).Failed())
            return EZ_FAILURE;
      }
      else
      {
        ezLog::Error("Unexpected material metadata group '{0}'.", sToken);
        return EZ_FAILURE;
      }
    }

    return EZ_SUCCESS;
  }
} // namespace RPI
