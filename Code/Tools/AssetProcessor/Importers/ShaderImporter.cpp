// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#include <AssetProcessor/Importers/ShaderImporter.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>

#include <mpack/mpack.h>

using namespace RAI;

static ezEnum<RHI::spInputElementLocationSemantic> GetSemanticLocationFromName(const ezStringView& sName)
{
  if (sName.Compare("POSITION") == 0)
    return RHI::spInputElementLocationSemantic::Position;
  if (sName.Compare("NORMAL") == 0)
    return RHI::spInputElementLocationSemantic::Normal;
  if (sName.Compare("TANGENT") == 0)
    return RHI::spInputElementLocationSemantic::Tangent;
  if (sName.Compare("TEXCOORD") == 0)
    return RHI::spInputElementLocationSemantic::TexCoord;
  if (sName.Compare("COLOR") == 0)
    return RHI::spInputElementLocationSemantic::Color;
  if (sName.Compare("BITANGENT") == 0)
    return RHI::spInputElementLocationSemantic::BiTangent;
  if (sName.Compare("BONEWEIGHTS") == 0)
    return RHI::spInputElementLocationSemantic::BoneWeights;
  if (sName.Compare("BONEINDICES") == 0)
    return RHI::spInputElementLocationSemantic::BoneIndices;

  return RHI::spInputElementLocationSemantic::Last;
}

static ezEnum<RHI::spInputElementFormat> GetFormatFromSemantic(const ezEnum<RHI::spInputElementLocationSemantic>& eSemantic)
{
  switch (eSemantic.GetValue())
  {
    case RHI::spInputElementLocationSemantic::TexCoord:
      return RHI::spInputElementFormat::Float2;
    case RHI::spInputElementLocationSemantic::Position:
    case RHI::spInputElementLocationSemantic::Normal:
      return RHI::spInputElementFormat::Float3;
    case RHI::spInputElementLocationSemantic::BoneIndices:
      return RHI::spInputElementFormat::UInt4;
    default:
    case RHI::spInputElementLocationSemantic::Tangent:
    case RHI::spInputElementLocationSemantic::BiTangent:
    case RHI::spInputElementLocationSemantic::Color:
    case RHI::spInputElementLocationSemantic::BoneWeights:
      return RHI::spInputElementFormat::Float4;
  }
}

static ezEnum<RHI::spShaderStage> GetShaderStageFromIndex(RHI::spShaderStage::StorageType sIndex)
{
  switch (sIndex)
  {
    case 1:
      return RHI::spShaderStage::VertexShader;
    case 2:
      return RHI::spShaderStage::PixelShader;
    case 3:
      return RHI::spShaderStage::GeometryShader;
    case 4:
      return RHI::spShaderStage::HullShader;
    case 5:
      return RHI::spShaderStage::DomainShader;
    case 6:
      return RHI::spShaderStage::ComputeShader;
    default:
      return RHI::spShaderStage::None;
  }
}

static void mpack_node_hstr(const mpack_node_t& node, ezHashedString& out_str)
{
  const ezUInt32 len = mpack_node_strlen(node);

  ezDynamicArray<char> str;
  str.SetCountUninitialized(len + 1);

  mpack_node_copy_cstr(node, str.GetData(), str.GetCount());

  out_str.Assign(str.GetData());
}

static ezResult DeserializeInputElement(const mpack_node_t& root, RHI::spInputElementDescription& ref_element)
{
  static constexpr ezUInt32 kSemanticNameIndex = 0;
  static constexpr ezUInt32 kSemanticIndex = 1;
  static constexpr ezUInt32 kSemanticFormat = 2;

  // Nodes
  const mpack_node_t& semanticNameNode = mpack_node_array_at(root, kSemanticNameIndex);
  const mpack_node_t& semanticIndexNode = mpack_node_array_at(root, kSemanticIndex);
  const mpack_node_t& semanticFormatNode = mpack_node_array_at(root, kSemanticFormat);

  if (mpack_node_is_missing(semanticNameNode) || mpack_node_is_missing(semanticIndexNode) || mpack_node_is_missing(semanticFormatNode))
    return EZ_FAILURE;

  // Element name
  mpack_node_hstr(semanticNameNode, ref_element.m_sName);

  // Semantic
  ref_element.m_eSemantic = GetSemanticLocationFromName(ref_element.m_sName);

  // Format
  ref_element.m_eFormat = GetFormatFromSemantic(ref_element.m_eSemantic);

  return EZ_SUCCESS;
}

static ezResult DeserializeShader(const mpack_node_t& root, spShaderVariant& ref_data)
{
  static constexpr ezUInt32 kShaderNameIndex = 0;
  static constexpr ezUInt32 kShaderInputElementsIndex = 1;
  static constexpr ezUInt32 kShaderSamplersIndex = 2;
  static constexpr ezUInt32 kShaderByteCodeIndex = 3;
  static constexpr ezUInt32 kShaderEntryPointsIndex = 4;
  static constexpr ezUInt32 kShaderPermutationsIndex = 5;
  static constexpr ezUInt32 kShaderStateIndex = 6;
  static constexpr ezUInt32 kShaderLangIndex = 7;

  // Nodes
  const mpack_node_t& shaderNameNode = mpack_node_array_at(root, kShaderNameIndex);
  const mpack_node_t& shaderInputElementsNode = mpack_node_array_at(root, kShaderInputElementsIndex);
  const mpack_node_t& shaderSamplersNode = mpack_node_array_at(root, kShaderSamplersIndex);
  const mpack_node_t& shaderByteCodeNode = mpack_node_array_at(root, kShaderByteCodeIndex);
  const mpack_node_t& shaderEntryPointsNode = mpack_node_array_at(root, kShaderEntryPointsIndex);
  const mpack_node_t& shaderPermutationsNode = mpack_node_array_at(root, kShaderPermutationsIndex);
  const mpack_node_t& shaderStateNode = mpack_node_array_at(root, kShaderStateIndex);
  const mpack_node_t& shaderLangNode = mpack_node_array_at(root, kShaderLangIndex);

  if (mpack_node_is_missing(shaderNameNode) ||
      mpack_node_is_missing(shaderInputElementsNode) ||
      mpack_node_is_missing(shaderSamplersNode) ||
      mpack_node_is_missing(shaderByteCodeNode) ||
      mpack_node_is_missing(shaderEntryPointsNode) ||
      mpack_node_is_missing(shaderPermutationsNode) ||
      mpack_node_is_missing(shaderStateNode) ||
      mpack_node_is_missing(shaderLangNode))
    return EZ_FAILURE;

  // Material name
  mpack_node_hstr(shaderNameNode, ref_data.m_sName);

  // Input elements
  {
    const ezUInt64 uiElementCount = mpack_node_array_length(shaderInputElementsNode);
    ref_data.m_InputElements.SetCount(uiElementCount);

    for (ezUInt64 i = 0; i < uiElementCount; ++i)
    {
      const mpack_node_t& element = mpack_node_array_at(shaderInputElementsNode, i);
      if (DeserializeInputElement(element, ref_data.m_InputElements[i]).Failed())
        return EZ_FAILURE;
    }
  }

  // Samplers
  {
    // TODO
  }

  // Byte code
  {
    const mpack_node_t& byteCodeNode = mpack_node_array_at(shaderByteCodeNode, 0);
    ezUInt64 uiByteCodeSize = mpack_node_bin_size(byteCodeNode);
    const char* szByteCode = mpack_node_bin_data(byteCodeNode);

    ref_data.m_Buffer.SetCountUninitialized(uiByteCodeSize);
    ezMemoryUtils::RawByteCopy(ref_data.m_Buffer.GetData(), szByteCode, uiByteCodeSize);
  }

  // Entry points
  {
    const ezUInt64 uiEntryPointCount = mpack_node_map_count(shaderEntryPointsNode);

    for (ezUInt64 i = 0; i < uiEntryPointCount; ++i)
    {
      const mpack_node_t& shaderStage = mpack_node_map_key_at(shaderEntryPointsNode, i);
      const mpack_node_t& entryPoint = mpack_node_map_value_at(shaderEntryPointsNode, i);

      if (mpack_node_is_missing(shaderStage) || mpack_node_is_missing(entryPoint))
        return EZ_FAILURE;

      const ezUInt32 uiShaderStageIndex = mpack_node_int(shaderStage);

      ezHashedString sEntryPointName;
      mpack_node_hstr(entryPoint, sEntryPointName);

      ref_data.m_EntryPoints.Insert(GetShaderStageFromIndex(uiShaderStageIndex), sEntryPointName);
    }
  }

  // Permutations
  {
    const ezUInt64 uiPermutationCount = mpack_node_map_count(shaderPermutationsNode);
    ref_data.m_Permutations.SetCount(uiPermutationCount);

    for (ezUInt64 i = 0; i < uiPermutationCount; ++i)
    {
      const mpack_node_t& permutationNameNode = mpack_node_map_key_at(shaderPermutationsNode, i);
      const mpack_node_t& permutationValueNode = mpack_node_map_value_at(shaderPermutationsNode, i);

      if (mpack_node_is_missing(permutationNameNode) || mpack_node_is_missing(permutationValueNode))
        return EZ_FAILURE;

      auto& permutation = ref_data.m_Permutations[i];

      mpack_node_hstr(permutationNameNode, permutation.m_sName);
      mpack_node_hstr(permutationValueNode, permutation.m_sValue);
    }
  }

  // State
  {
    enum class StateValueMode : ezUInt8
    {
      Map = 1,
      String = 2,
    };

    const ezUInt64 uiStateCount = mpack_node_array_length(shaderStateNode);

    for (ezUInt64 i = 0; i < uiStateCount; ++i)
    {
      const mpack_node_t& stateNode = mpack_node_array_at(shaderStateNode, i);

      if (mpack_node_is_missing(stateNode))
        return EZ_FAILURE;

      const mpack_node_t& stateNameNode = mpack_node_array_at(stateNode, 0);
      const mpack_node_t& stateMapNode = mpack_node_array_at(stateNode, 1);
      const mpack_node_t& stateValueNode = mpack_node_array_at(stateNode, 2);
      const mpack_node_t& stateTypeNode = mpack_node_array_at(stateNode, 3);

      if (mpack_node_is_missing(stateNameNode) || mpack_node_is_missing(stateMapNode) || mpack_node_is_missing(stateValueNode) || mpack_node_is_missing(stateTypeNode))
        return EZ_FAILURE;

      const ezStringView sStateName(mpack_node_str(stateNameNode), mpack_node_strlen(stateNameNode));
      const auto eStateType = static_cast<StateValueMode>(mpack_node_u8(stateTypeNode));

      if (sStateName.Compare("BlendState") == 0)
      {
        switch (eStateType)
        {
          case StateValueMode::Map:
          {
            ref_data.m_RenderingState.m_BlendState.m_AttachmentStates.SetCount(1);

            const ezUInt64 uiMapSize = mpack_node_map_count(stateMapNode);

            for (ezUInt64 j = 0; j < uiMapSize; ++j)
            {
              const mpack_node_t& keyNode = mpack_node_map_key_at(stateMapNode, j);
              const mpack_node_t& valueNode = mpack_node_map_value_at(stateMapNode, j);

              const ezStringView sKey(mpack_node_str(keyNode), mpack_node_strlen(keyNode));
              const ezStringView sValue(mpack_node_str(valueNode), mpack_node_strlen(valueNode));

              if (sKey.Compare("BlendColor") == 0)
              {
                bool bValidColor = false;
                ref_data.m_RenderingState.m_BlendState.m_BlendColor = ezConversionUtils::GetColorByName(sValue, &bValidColor);
                if (!bValidColor)
                  ezLog::Error("Invalid blend color value: '{0}'", sValue);
              }
              else if (sValue.Compare("AlphaToCoverage") == 0)
                ref_data.m_RenderingState.m_BlendState.m_bAlphaToCoverage = sValue.Compare("true") == 0;
              else if (sKey.Compare("ColorWriteMask") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eColorWriteMask = RHI::spColorWriteMask::FromString(sValue);
              else if (sValue.Compare("SourceColorBlendFactor") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eSourceColorBlendFactor = RHI::spBlendFactor::FromString(sValue);
              else if (sValue.Compare("DestinationColorBlendFactor") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eDestinationColorBlendFactor = RHI::spBlendFactor::FromString(sValue);
              else if (sValue.Compare("SourceAlphaBlendFactor") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eSourceAlphaBlendFactor = RHI::spBlendFactor::FromString(sValue);
              else if (sValue.Compare("DestinationAlphaBlendFactor") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eDestinationAlphaBlendFactor = RHI::spBlendFactor::FromString(sValue);
              else if (sValue.Compare("ColorBlendFunction") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eColorBlendFunction = RHI::spBlendFunction::FromString(sValue);
              else if (sValue.Compare("AlphaBlendFunction") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_eAlphaBlendFunction = RHI::spBlendFunction::FromString(sValue);
              else if (sValue.Compare("Enabled") == 0)
                ref_data.m_RenderingState.m_BlendState.m_AttachmentStates[0].m_bEnabled = sValue.Compare("true") == 0;
              else
                ezLog::Warning("Found unknown blend state key: '{0}'", sKey);
            }

            break;
          }

          case StateValueMode::String:
          {
            const ezStringView sStateValue(mpack_node_str(stateValueNode), mpack_node_strlen(stateValueNode));

            if (sStateValue.Compare("Empty") == 0)
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::Empty;
            else if (sStateValue.Compare("SingleOverrideBlend") == 0)
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::SingleOverrideBlend;
            else if (sStateValue.Compare("SingleAlphaBlend") == 0)
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::SingleAlphaBlend;
            else if (sStateValue.Compare("SingleAdditiveBlend") == 0)
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::SingleAdditiveBlend;
            else if (sStateValue.Compare("SingleMultiplyBlend") == 0)
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::SingleMultiplyBlend;
            else if (sStateValue.Compare("SingleDisabled") == 0)
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::SingleDisabled;
            else
            {
              ezLog::Error("Unknown blend state value '{0}'. Falling back to 'SingleDisabled'", sStateValue);
              ref_data.m_RenderingState.m_BlendState = RHI::spBlendState::SingleDisabled;
            }

            break;
          }
        }
      }
      else if (sStateName.Compare("DepthState") == 0)
      {
        switch (eStateType)
        {
          case StateValueMode::Map:
          {
            const ezUInt64 uiMapSize = mpack_node_map_count(stateMapNode);

            for (ezUInt64 j = 0; j < uiMapSize; ++j)
            {
              const mpack_node_t& keyNode = mpack_node_map_key_at(stateMapNode, j);
              const mpack_node_t& valueNode = mpack_node_map_value_at(stateMapNode, j);

              const ezStringView sKey(mpack_node_str(keyNode), mpack_node_strlen(keyNode));
              const ezStringView sValue(mpack_node_str(valueNode), mpack_node_strlen(valueNode));

              if (sKey.Compare("DepthTestEnabled") == 0)
                ref_data.m_RenderingState.m_DepthState.m_bDepthTestEnabled = sValue.Compare("true") == 0;
              else if (sKey.Compare("DepthMaskEnabled") == 0)
                ref_data.m_RenderingState.m_DepthState.m_bDepthMaskEnabled = sValue.Compare("true") == 0;
              else if (sKey.Compare("DepthStencilComparison") == 0)
                ref_data.m_RenderingState.m_DepthState.m_eDepthStencilComparison = RHI::spDepthStencilComparison::FromString(sValue);
              else
                ezLog::Warning("Found unknown depth state key: '{0}'", sKey);
            }

            break;
          }

          case StateValueMode::String:
          {
            const ezStringView sStateValue(mpack_node_str(stateValueNode), mpack_node_strlen(stateValueNode));

            if (sStateValue.Compare("Less") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::Less;
            else if (sStateValue.Compare("LessEqual") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::LessEqual;
            else if (sStateValue.Compare("LessRead") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::LessRead;
            else if (sStateValue.Compare("LessEqualRead") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::LessEqualRead;
            else if (sStateValue.Compare("Greater") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::Greater;
            else if (sStateValue.Compare("GreaterEqual") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::GreaterEqual;
            else if (sStateValue.Compare("GreaterRead") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::GreaterRead;
            else if (sStateValue.Compare("GreaterEqualRead") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::GreaterEqualRead;
            else if (sStateValue.Compare("Disabled") == 0)
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::Disabled;
            else
            {
              ezLog::Error("Unknown depth state value '{0}'. Falling back to 'Disabled'", sStateValue);
              ref_data.m_RenderingState.m_DepthState = RHI::spDepthState::Disabled;
            }

            break;
          }
        }
      }
      else if (sStateName.Compare("StencilState") == 0)
      {
        switch (eStateType)
        {
          case StateValueMode::Map:
          {
            const ezUInt64 uiMapSize = mpack_node_map_count(stateMapNode);

            for (ezUInt64 j = 0; j < uiMapSize; ++j)
            {
              const mpack_node_t& keyNode = mpack_node_map_key_at(stateMapNode, j);
              const mpack_node_t& valueNode = mpack_node_map_value_at(stateMapNode, j);

              const ezStringView sKey(mpack_node_str(keyNode), mpack_node_strlen(keyNode));
              const ezStringView sValue(mpack_node_str(valueNode), mpack_node_strlen(valueNode));

              if (sKey.Compare("Enabled") == 0)
                ref_data.m_RenderingState.m_StencilState.m_bEnabled = sValue.Compare("true") == 0;
              else if (sKey.Compare("FrontFailOp") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Front.m_eFail = RHI::spStencilOperation::FromString(sValue);
              else if (sKey.Compare("FrontPassOp") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Front.m_ePass = RHI::spStencilOperation::FromString(sValue);
              else if (sKey.Compare("FrontDepthFailOp") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Front.m_eDepthFail = RHI::spStencilOperation::FromString(sValue);
              else if (sKey.Compare("FrontComparison") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Front.m_eComparison = RHI::spDepthStencilComparison::FromString(sValue);
              else if (sKey.Compare("BackFailOp") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Back.m_eFail = RHI::spStencilOperation::FromString(sValue);
              else if (sKey.Compare("BackPassOp") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Back.m_ePass = RHI::spStencilOperation::FromString(sValue);
              else if (sKey.Compare("BackDepthFailOp") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Back.m_eDepthFail = RHI::spStencilOperation::FromString(sValue);
              else if (sKey.Compare("BackComparison") == 0)
                ref_data.m_RenderingState.m_StencilState.m_Back.m_eComparison = RHI::spDepthStencilComparison::FromString(sValue);
              else if (sKey.Compare("ReadMask") == 0)
              {
                ezUInt32 uiReadMask = 0;
                if (ezConversionUtils::StringToUInt(sValue, uiReadMask).Failed())
                {
                  ezLog::Error("Invalid read mask '{0}'", sValue);
                  uiReadMask = 0;
                }

                ref_data.m_RenderingState.m_StencilState.m_uiReadMask = static_cast<ezUInt8>(uiReadMask);
              }
              else if (sKey.Compare("WriteMask") == 0)
              {
                ezUInt32 uiWriteMask = 0;
                if (ezConversionUtils::StringToUInt(sValue, uiWriteMask).Failed())
                {
                  ezLog::Error("Invalid write mask '{0}'", sValue);
                  uiWriteMask = 0;
                }

                ref_data.m_RenderingState.m_StencilState.m_uiWriteMask = static_cast<ezUInt8>(uiWriteMask);
              }
              else if (sKey.Compare("Reference") == 0)
              {
                ezUInt32 uiReference = 0;
                if (ezConversionUtils::StringToUInt(sValue, uiReference).Failed())
                {
                  ezLog::Error("Invalid reference value '{0}'", sValue);
                  uiReference = 0;
                }

                ref_data.m_RenderingState.m_StencilState.m_uiReference = static_cast<ezUInt8>(uiReference);
              }
              else
                ezLog::Warning("Found unknown depth state key: '{0}'", sKey);
            }

            break;
          }

          case StateValueMode::String:
          {
            const ezStringView sStateValue(mpack_node_str(stateValueNode), mpack_node_strlen(stateValueNode));

            if (sStateValue.Compare("Disabled") == 0)
              ref_data.m_RenderingState.m_StencilState = RHI::spStencilState::Disabled;
            else
            {
              ezLog::Error("Unknown depth state value '{0}'. Falling back to 'Disabled'", sStateValue);
              ref_data.m_RenderingState.m_StencilState = RHI::spStencilState::Disabled;
            }

            break;
          }
        }
      }
      else if (sStateName.Compare("RasterizerState") == 0)
      {
        switch (eStateType)
        {
          case StateValueMode::Map:
          {
            const ezUInt64 uiMapSize = mpack_node_map_count(stateMapNode);

            for (ezUInt64 j = 0; j < uiMapSize; ++j)
            {
              const mpack_node_t& keyNode = mpack_node_map_key_at(stateMapNode, j);
              const mpack_node_t& valueNode = mpack_node_map_value_at(stateMapNode, j);

              const ezStringView sKey(mpack_node_str(keyNode), mpack_node_strlen(keyNode));
              const ezStringView sValue(mpack_node_str(valueNode), mpack_node_strlen(valueNode));

              if (sKey.Compare("FaceCulling") == 0)
                ref_data.m_RenderingState.m_RasterizerState.m_eFaceCulling = RHI::spFaceCullMode::FromString(sValue);
              else if (sKey.Compare("ScissorTestEnabled") == 0)
                ref_data.m_RenderingState.m_RasterizerState.m_bScissorTestEnabled = sValue.Compare("true") == 0;
              else if (sKey.Compare("DepthClipEnabled") == 0)
                ref_data.m_RenderingState.m_RasterizerState.m_bDepthClipEnabled = sValue.Compare("true") == 0;
              else if (sKey.Compare("FrontFace") == 0)
                ref_data.m_RenderingState.m_RasterizerState.m_eFrontFace = RHI::spFrontFace::FromString(sValue);
              else if (sKey.Compare("PolygonFillMode") == 0)
                ref_data.m_RenderingState.m_RasterizerState.m_ePolygonFillMode = RHI::spPolygonFillMode::FromString(sValue);
              else if (sKey.Compare("ConservativeRasterization") == 0)
                ref_data.m_RenderingState.m_RasterizerState.m_bConservativeRasterization = sValue.Compare("true") == 0;
              else
                ezLog::Warning("Found unknown rasterizer state key: '{0}'", sKey);
            }

            break;
          }

          case StateValueMode::String:
          {
            const ezStringView sStateValue(mpack_node_str(stateValueNode), mpack_node_strlen(stateValueNode));

            if (sStateValue.Compare("Default") == 0)
              ref_data.m_RenderingState.m_RasterizerState = RHI::spRasterizerState::Default;
            else if (sStateValue.Compare("CullNone") == 0)
              ref_data.m_RenderingState.m_RasterizerState = RHI::spRasterizerState::CullNone;
            else
            {
              ezLog::Error("Unknown rasterizer state value '{0}'. Falling back to 'Default'", sStateValue);
              ref_data.m_RenderingState.m_RasterizerState = RHI::spRasterizerState::Default;
            }

            break;
          }
        }
      }
      else
      {
        ezLog::Warning("Found unknown rendering state key: '{0}'", sStateName);
      }
    }
  }

  // Lang
  {
    ref_data.m_eShaderLanguage = static_cast<RHI::spShaderLanguage::Enum>(mpack_node_u8(shaderLangNode));
  }

  return EZ_SUCCESS;
}

ezResult spShaderImporter::Import(ezStringView sAssetPath, ezStringView sOutputPath)
{
  spShaderVariantResourceDescriptor desc;

  // Parse shader variant file
  {
    ezFileReader file;
    if (file.Open(sAssetPath).Failed())
      return EZ_FAILURE;

    if (!sAssetPath.HasExtension("spslb"))
      return EZ_FAILURE;

    ezDynamicArray<ezUInt8> content;
    content.SetCountUninitialized(file.GetFileSize());

    file.ReadBytes(content.GetData(), content.GetCount());

    mpack_tree_t tree;
    mpack_tree_init_data(&tree, reinterpret_cast<const char*>(content.GetData()), content.GetCount());
    mpack_tree_parse(&tree);

    const mpack_node_t& root = mpack_tree_root(&tree);

    if (DeserializeShader(root, desc.GetShaderVariant()).Failed())
      return EZ_FAILURE;

    if (mpack_tree_destroy(&tree) != mpack_ok)
      return EZ_FAILURE;
  }

  ezStringBuilder sOutputFile(sOutputPath);
  sOutputFile.AppendFormat("/{}.spShaderVariant", sAssetPath.GetFileName());

  // Write shader variant resource
  {
    ezFileWriter file;
    if (file.Open(sOutputFile, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to save mesh asset: '{0}'", sOutputFile);
      return EZ_FAILURE;
    }

    // Write asset header
    ezAssetFileHeader assetHeader;
    assetHeader.SetGenerator("SparkEngine Asset Processor");
    assetHeader.SetFileHashAndVersion(ezHashingUtils::xxHash64String(sAssetPath), 1);
    EZ_SUCCEED_OR_RETURN(assetHeader.Write(file));

    EZ_SUCCEED_OR_RETURN(desc.Save(file));
  }

  return EZ_SUCCESS;
}

spShaderImporter::spShaderImporter(const spShaderVariantImporterConfiguration& config)
  : spImporter<spShaderVariantImporterConfiguration>(config)
{
}
