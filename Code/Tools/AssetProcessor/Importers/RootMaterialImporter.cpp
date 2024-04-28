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

#include <AssetProcessor/Importers/RootMaterialImporter.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#include <mpack/mpack.h>

using namespace RAI;

static ezResult DeserializePermutation(const mpack_node_t& node, spRootMaterial::Permutation& ref_parameter)
{
  static constexpr ezUInt32 kNameIndex = 0;
  static constexpr ezUInt32 kTypeIndex = 1;
  static constexpr ezUInt32 kValuesIndex = 2;

  // Nodes
  const mpack_node_t& nameNode = mpack_node_array_at(node, kNameIndex);
  const mpack_node_t& typeNode = mpack_node_array_at(node, kTypeIndex);
  const mpack_node_t& valuesNode = mpack_node_array_at(node, kValuesIndex);

  // Name
  mpack_node_hstr(nameNode, ref_parameter.m_sName);

  // Type
  ref_parameter.m_eType = static_cast<spRootMaterial::PermutationType::Enum>(mpack_node_u8(typeNode));

  // Values
  const ezUInt64 uiValuesCount = mpack_node_array_length(valuesNode);
  ref_parameter.m_AllowedValues.SetCount(uiValuesCount);

  for (ezUInt32 i = 0; i < uiValuesCount; ++i)
  {
    ref_parameter.m_AllowedValues[i] = mpack_node_str(mpack_node_array_at(valuesNode, i));
  }

  return EZ_SUCCESS;
}

static ezResult DeserializeParameterDataType(const mpack_node_t& node, spRootMaterial::ParameterDataType& ref_parameter)
{
  static constexpr ezUInt32 kTypeIndex = 0;
  static constexpr ezUInt32 kRowCountIndex = 1;
  static constexpr ezUInt32 kColumnCountIndex = 2;
  static constexpr ezUInt32 kArraySizeIndex = 3;

  // Nodes
  const mpack_node_t& typeNode = mpack_node_array_at(node, kTypeIndex);
  const mpack_node_t& rowCountNode = mpack_node_array_at(node, kRowCountIndex);
  const mpack_node_t& columnCountNode = mpack_node_array_at(node, kColumnCountIndex);
  const mpack_node_t& arraySizeNode = mpack_node_array_at(node, kArraySizeIndex);

  // Type
  ref_parameter.m_eType = static_cast<spRootMaterial::ParameterType::Enum>(mpack_node_u8(typeNode));

  // Row count
  ref_parameter.m_uiRowCount = mpack_node_u32(rowCountNode);

  // Column count
  ref_parameter.m_uiColumnCount = mpack_node_u32(columnCountNode);

  // Array size
  ref_parameter.m_uiArraySize = mpack_node_u32(arraySizeNode);

  return EZ_SUCCESS;
}

static ezResult DeserializeParameter(const mpack_node_t& node, spRootMaterial::Parameter& ref_parameter)
{
  static constexpr ezUInt32 kNameIndex = 0;
  static constexpr ezUInt32 kTypeIndex = 1;
  static constexpr ezUInt32 kDataTypeIndex = 2;

  // Nodes
  const mpack_node_t& nameNode = mpack_node_array_at(node, kNameIndex);
  const mpack_node_t& typeNode = mpack_node_array_at(node, kTypeIndex);
  const mpack_node_t& dataTypeNode = mpack_node_array_at(node, kDataTypeIndex);

  // Name
  mpack_node_hstr(nameNode, ref_parameter.m_sName);

  // Type
  ref_parameter.m_bIsPermutation = mpack_node_u8(typeNode) == 0;

  // Data type
  return DeserializeParameterDataType(dataTypeNode, ref_parameter.m_DataType);
}

static ezResult DeserializeParameterGroup(const mpack_node_t& node, spRootMaterial::ParameterGroup& ref_group)
{
  static constexpr ezUInt32 kNameIndex = 0;
  static constexpr ezUInt32 kParametersIndex = 1;

  // Nodes
  const mpack_node_t& nameNode = mpack_node_array_at(node, kNameIndex);
  const mpack_node_t& parametersNode = mpack_node_array_at(node, kParametersIndex);

  if (mpack_node_is_missing(nameNode) || mpack_node_is_missing(parametersNode))
    return EZ_FAILURE;

  // Name
  mpack_node_hstr(nameNode, ref_group.m_sName);

  // Parameters
  const ezUInt64 uiParametersCount = mpack_node_array_length(parametersNode);
  ref_group.m_Parameters.SetCount(uiParametersCount);

  for (ezUInt32 i = 0; i < uiParametersCount; ++i)
  {
    const mpack_node_t& parameterNode = mpack_node_array_at(parametersNode, i);
    if (DeserializeParameter(parameterNode, ref_group.m_Parameters[i]).Failed())
      return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

static ezResult DeserializeRootMaterial(const mpack_node_t& root, spRootMaterial& ref_data)
{
  static constexpr ezUInt32 kRootMaterialVersionIndex = 0;
  static constexpr ezUInt32 kRootMaterialNameIndex = 1;
  static constexpr ezUInt32 kRootMaterialParameterGroupsIndex = 2;
  static constexpr ezUInt32 kRootMaterialPermutationsIndex = 3;

  // Nodes
  const mpack_node_t& rootMaterialVersionNode = mpack_node_array_at(root, kRootMaterialVersionIndex);
  const mpack_node_t& rootMaterialNameNode = mpack_node_array_at(root, kRootMaterialNameIndex);
  const mpack_node_t& rootMaterialParameterGroupsNode = mpack_node_array_at(root, kRootMaterialParameterGroupsIndex);
  const mpack_node_t& rootMaterialPermutationsNode = mpack_node_array_at(root, kRootMaterialPermutationsIndex);

  if (mpack_node_is_missing(rootMaterialVersionNode))
    return EZ_FAILURE;

  // Version check
  ezUInt32 uiVersion = mpack_node_u32(rootMaterialVersionNode);

  if (uiVersion == 1)
  {
    if (mpack_node_is_missing(rootMaterialNameNode) || mpack_node_is_missing(rootMaterialParameterGroupsNode) || mpack_node_is_missing(rootMaterialPermutationsNode))
      return EZ_FAILURE;
  }

  // Root Material Name
  mpack_node_hstr(rootMaterialNameNode, ref_data.m_sName);

  // Parameter Groups
  {
    const ezUInt64 uiNumParameterGroups = mpack_node_array_length(rootMaterialParameterGroupsNode);
    ref_data.m_ParameterGroups.SetCount(uiNumParameterGroups);

    for (ezUInt64 i = 0; i < uiNumParameterGroups; ++i)
    {
      const mpack_node_t& parameterGroupNode = mpack_node_array_at(rootMaterialParameterGroupsNode, i);
      spRootMaterial::ParameterGroup& ref_parameterGroup = ref_data.m_ParameterGroups[i];

      if (DeserializeParameterGroup(parameterGroupNode, ref_parameterGroup).Failed())
        return EZ_FAILURE;
    }
  }

  // Permutations
  {
    const ezUInt64 uiNumPermutations = mpack_node_array_length(rootMaterialPermutationsNode);
    ref_data.m_Permutations.SetCount(uiNumPermutations);

    for (ezUInt64 i = 0; i < uiNumPermutations; ++i)
    {
      const mpack_node_t& permutationNode = mpack_node_array_at(rootMaterialPermutationsNode, i);
      if (DeserializePermutation(permutationNode, ref_data.m_Permutations[i]).Failed())
        return EZ_FAILURE;
    }
  }

  return EZ_SUCCESS;
}

ezResult spRootMaterialImporter::Import(ezStringView sAssetPath, ezStringView sOutputPath)
{
  spRootMaterialResourceDescriptor desc;

  // Parse root material file
  {
    if (!sAssetPath.HasExtension("sprm"))
      return EZ_FAILURE;

    ezFileReader file;
    if (file.Open(sAssetPath).Failed())
      return EZ_FAILURE;

    ezUInt8 header[4];
    file.ReadBytes(header, 4);
    if (header[0] != 'S' || header[1] != 'P' || header[2] != 'R' || header[3] != 'M')
      return EZ_FAILURE;

    ezDynamicArray<ezUInt8> content;
    content.SetCountUninitialized(file.GetFileSize() - sizeof(header));

    file.ReadBytes(content.GetData(), content.GetCount());

    mpack_tree_t tree;
    mpack_tree_init_data(&tree, reinterpret_cast<const char*>(content.GetData()), content.GetCount());
    mpack_tree_parse(&tree);

    const mpack_node_t& root = mpack_tree_root(&tree);

    if (DeserializeRootMaterial(root, desc.GetRootMaterial()).Failed())
      return EZ_FAILURE;

    if (mpack_tree_destroy(&tree) != mpack_ok)
      return EZ_FAILURE;
  }

  ezStringBuilder sOutputFile(sOutputPath);
  sOutputFile.AppendFormat("/{}.spRootMaterial", sAssetPath.GetFileName());

  // Write root material resource
  {
    ezFileWriter file;
    if (file.Open(sOutputFile).Failed())
    {
      ezLog::Error("Failed to save root material resource file: '{0}'", sOutputFile);
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

spRootMaterialImporter::spRootMaterialImporter(const spRootMaterialImporterConfiguration& config)
  : spImporter<spRootMaterialImporterConfiguration>(config)
{
}
