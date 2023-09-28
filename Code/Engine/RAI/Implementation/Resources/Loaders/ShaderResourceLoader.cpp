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

#include <RAI/RAIPCH.h>

#include <RAI/Resources/Loaders/ShaderResourceLoader.h>
#include <RAI/Resources/ShaderResource.h>

#include <Core/Assets/AssetFileHeader.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>

#include <mpack/mpack.h>

using namespace RAI;

static spShaderResourceLoader s_ShaderResourceLoader;

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RAI, ShaderResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezResourceManager::SetResourceTypeLoader<spShaderVariantResource>(&s_ShaderResourceLoader);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezResourceManager::SetResourceTypeLoader<spShaderVariantResource>(nullptr);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

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

ezResourceLoadData spShaderResourceLoader::OpenDataStream(const ezResource* pResource)
{
  LoadedData* pLoadedData = EZ_DEFAULT_NEW(LoadedData);

  ezResourceLoadData res;

  ezFileReader file;
  if (file.Open(pResource->GetResourceID().GetData()).Failed())
    return res;

  const ezStringBuilder sAbsolutePath = file.GetFilePathAbsolute();
  res.m_sResourceDescription = file.GetFilePathRelative().GetView();

#if EZ_ENABLED(EZ_SUPPORTS_FILE_STATS)
  {
    ezFileStats stat;
    if (ezFileSystem::GetFileStats(pResource->GetResourceID(), stat).Succeeded())
    {
      res.m_LoadedFileModificationDate = stat.m_LastModificationTime;
    }
  }
#endif

  if (!sAbsolutePath.HasExtension("spslb"))
    return res;

  {
    ezStringBuilder content;
    content.ReadAll(file);

    mpack_tree_t tree;
    mpack_tree_init_data(&tree, content.GetData(), content.GetElementCount());
    mpack_tree_parse(&tree);

    mpack_node_t root = mpack_tree_root(&tree);

    if (DeserializeShader(root, *pLoadedData).Failed())
      return res;

    if (mpack_tree_destroy(&tree) != mpack_ok)
      return res;
  }

  ezMemoryStreamWriter w(&pLoadedData->m_Storage);

  if (w.WriteBytes(&pLoadedData, sizeof(LoadedData*)).Failed())
    return res;

  res.m_pDataStream = &pLoadedData->m_Reader;
  res.m_pCustomLoaderData = pLoadedData;

  return res;
}

void spShaderResourceLoader::CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData)
{
  auto* pLoadedData = static_cast<LoadedData*>(loaderData.m_pCustomLoaderData);

  pLoadedData->m_InputElements.Clear();
  pLoadedData->m_Buffer.Clear();
  pLoadedData->m_EntryPoints.Clear();
  pLoadedData->m_Permutations.Clear();

  EZ_DEFAULT_DELETE(pLoadedData);
}

bool spShaderResourceLoader::IsResourceOutdated(const ezResource* pResource) const
{
  // Don't try to reload a file that cannot be found
  ezStringBuilder sAbs;
  if (ezFileSystem::ResolvePath(pResource->GetResourceID(), &sAbs, nullptr).Failed())
    return false;

#if EZ_ENABLED(EZ_SUPPORTS_FILE_STATS)

  if (pResource->GetLoadedFileModificationTime().IsValid())
  {
    ezFileStats stat;
    if (ezFileSystem::GetFileStats(pResource->GetResourceID(), stat).Failed())
      return false;

    return !stat.m_LastModificationTime.Compare(pResource->GetLoadedFileModificationTime(), ezTimestamp::CompareMode::FileTimeEqual);
  }

#endif

  return true;
}

ezResult spShaderResourceLoader::DeserializeShader(mpack_node_t root, spShaderResourceLoader::LoadedData& ref_data)
{
  static constexpr ezUInt32 kShaderNameIndex = 0;
  static constexpr ezUInt32 kShaderInputElementsIndex = 1;
  static constexpr ezUInt32 kShaderSamplersIndex = 2;
  static constexpr ezUInt32 kShaderByteCodeIndex = 3;
  static constexpr ezUInt32 kShaderEntryPointsIndex = 4;
  static constexpr ezUInt32 kShaderPermutationsIndex = 5;
  static constexpr ezUInt32 kShaderStateIndex = 6;

  // Nodes
  mpack_node_t shaderNameNode = mpack_node_map_int(root, kShaderNameIndex);
  mpack_node_t shaderInputElementsNode = mpack_node_map_int(root, kShaderInputElementsIndex);
  mpack_node_t shaderSamplersNode = mpack_node_map_int(root, kShaderSamplersIndex);
  mpack_node_t shaderByteCodeNode = mpack_node_map_int(root, kShaderByteCodeIndex);
  mpack_node_t shaderEntryPointsNode = mpack_node_map_int(root, kShaderEntryPointsIndex);
  mpack_node_t shaderPermutationsNode = mpack_node_map_int(root, kShaderPermutationsIndex);
  mpack_node_t shaderStateNode = mpack_node_map_int(root, kShaderStateIndex);

  if (mpack_node_is_missing(shaderNameNode) ||
      mpack_node_is_missing(shaderInputElementsNode) ||
      mpack_node_is_missing(shaderSamplersNode) ||
      mpack_node_is_missing(shaderByteCodeNode) ||
      mpack_node_is_missing(shaderEntryPointsNode) ||
      mpack_node_is_missing(shaderPermutationsNode) ||
      mpack_node_is_missing(shaderStateNode))
    return EZ_FAILURE;

  // Material name
  {
    ref_data.m_sName = mpack_node_str(shaderNameNode);
  }

  // Input elements
  {
    const ezUInt64 uiElementCount = mpack_node_array_length(shaderInputElementsNode);
    ref_data.m_InputElements.SetCount(uiElementCount);

    for (ezUInt64 i = 0; i < uiElementCount; ++i)
    {
      mpack_node_t element = mpack_node_array_at(shaderInputElementsNode, i);
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
    mpack_node_t byteCodeNode = mpack_node_map_int(shaderByteCodeNode, 0);
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
      mpack_node_t shaderStage = mpack_node_map_key_at(shaderEntryPointsNode, i);
      mpack_node_t entryPoint = mpack_node_map_value_at(shaderEntryPointsNode, i);
      if (mpack_node_is_missing(shaderStage) || mpack_node_is_missing(entryPoint))
        return EZ_FAILURE;

      const ezUInt32 uiShaderStageIndex = mpack_node_int(shaderStage);
      const ezStringView sEntryPointName = mpack_node_str(entryPoint);

      ref_data.m_EntryPoints.Insert(GetShaderStageFromIndex(uiShaderStageIndex), sEntryPointName);
    }
  }

  // Permutations
  {
    const ezUInt64 uiPermutationCount = mpack_node_map_count(shaderPermutationsNode);
    ref_data.m_Permutations.SetCount(uiPermutationCount);

    for (ezUInt64 i = 0; i < uiPermutationCount; ++i)
    {
      mpack_node_t permutationNameNode = mpack_node_map_key_at(shaderPermutationsNode, i);
      mpack_node_t permutationValueNode = mpack_node_map_value_at(shaderPermutationsNode, i);
      if (mpack_node_is_missing(permutationNameNode) || mpack_node_is_missing(permutationValueNode))
        return EZ_FAILURE;

      auto& permutation = ref_data.m_Permutations[i];

      permutation.m_sName.Assign(mpack_node_str(permutationNameNode));
      permutation.m_sValue.Assign(mpack_node_str(permutationValueNode));
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
        mpack_node_t stateNode = mpack_node_array_at(shaderStateNode, i);
        if (mpack_node_is_missing(stateNode))
          return EZ_FAILURE;

        mpack_node_t stateNameNode = mpack_node_map_int(stateNode, 0);
        mpack_node_t stateMapNode = mpack_node_map_int(stateNode, 1);
        mpack_node_t stateValueNode = mpack_node_map_int(stateNode, 2);
        mpack_node_t stateTypeNode = mpack_node_map_int(stateNode, 3);

        if (mpack_node_is_missing(stateNameNode) || mpack_node_is_missing(stateMapNode) || mpack_node_is_missing(stateValueNode) || mpack_node_is_missing(stateTypeNode))
          return EZ_FAILURE;

        const ezStringView sStateName = mpack_node_str(stateNameNode);
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
                const ezStringView sKey = mpack_node_str(mpack_node_map_key_at(stateMapNode, j));
                const ezStringView sValue = mpack_node_str(mpack_node_map_value_at(stateMapNode, j));

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
              const ezStringView sStateValue = mpack_node_str(stateValueNode);

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
                const ezStringView sKey = mpack_node_str(mpack_node_map_key_at(stateMapNode, j));
                const ezStringView sValue = mpack_node_str(mpack_node_map_value_at(stateMapNode, j));

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
              const ezStringView sStateValue = mpack_node_str(stateValueNode);

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
                const ezStringView sKey = mpack_node_str(mpack_node_map_key_at(stateMapNode, j));
                const ezStringView sValue = mpack_node_str(mpack_node_map_value_at(stateMapNode, j));

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
              const ezStringView sStateValue = mpack_node_str(stateValueNode);

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
                const ezStringView sKey = mpack_node_str(mpack_node_map_key_at(stateMapNode, j));
                const ezStringView sValue = mpack_node_str(mpack_node_map_value_at(stateMapNode, j));

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
              const ezStringView sStateValue = mpack_node_str(stateValueNode);

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
  }

  return EZ_SUCCESS;
}

ezResult spShaderResourceLoader::DeserializeInputElement(mpack_node_t root, RHI::spInputElementDescription& ref_element)
{
  static constexpr ezUInt32 kSemanticNameIndex = 0;
  static constexpr ezUInt32 kSemanticIndex = 1;
  static constexpr ezUInt32 kSemanticFormat = 2;

  // Nodes
  mpack_node_t semanticNameNode = mpack_node_map_int(root, kSemanticNameIndex);
  mpack_node_t semanticIndexNode = mpack_node_map_int(root, kSemanticIndex);
  mpack_node_t semanticFormatNode = mpack_node_map_int(root, kSemanticFormat);

  if (mpack_node_is_missing(semanticNameNode) || mpack_node_is_missing(semanticIndexNode) || mpack_node_is_missing(semanticFormatNode))
    return EZ_FAILURE;

  // Element name
  ref_element.m_sName.Assign(mpack_node_str(semanticNameNode));

  // Semantic
  ref_element.m_eSemantic = GetSemanticLocationFromName(ref_element.m_sName);

  // Format
  ref_element.m_eFormat = GetFormatFromSemantic(ref_element.m_eSemantic);

  return EZ_SUCCESS;
}
