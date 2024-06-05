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

#pragma once

#include <AssetProcessor/Importers/Importer.h>

#include <RAI/Mesh.h>

#include <Foundation/Strings/HashedString.h>

#include <assimp/Importer.hpp>

struct aiAnimMesh;
struct aiMesh;
struct aiNode;

/// \brief Precision of a single component in a vertex stream.
struct spAssimpVertexStreamComponentPrecision
{
  typedef ezUInt8 StorageType;

  enum Enum : StorageType
  {
    /// \brief 8-bit precision. This represents an unsigned byte.
    _8Bit = 8,

    /// \brief 10-bit precision. This represents a part of a packed
    /// 32-bit unsigned integer.
    _10Bit = 10,

    /// \brief 16-bit precision. This represents an unsigned short.
    _16Bit = 16,

    /// \brief 32-bit precision. This represents a float.
    _32Bit = 32,

    Default = _32Bit,
  };
};

/// \brief Assimp Importer settings.
struct spAssimpImporterConfiguration
{
  /// \brief The global scale to apply on the imported mesh.
  float m_fScale{1.0f};

  /// \brief Specifies if the mesh was exported from a DCC tool using the Spark LOD system specifications.
  bool m_bHasLODs{false};

  /// \brief Whether to flip UVs.
  bool m_bFlipUVs{false};

  /// \brief Whether to recompute normals. Normals will be recalculated even
  /// if the mesh already have normals.
  bool m_bRecomputeNormals{false};

  /// \brief Whether to recompute tangent vectors. It's highly recommended to always
  /// enable this setting.
  bool m_bRecomputeTangents{false};

  /// \brief Whether to flip in-facing normal vectors.
  bool m_bFlipWindingNormals{false};

  /// \brief Whether to optimize the mesh for GPU drawing. This will reorder the index and
  /// vertex buffers to reduce the number of data fetch from the GPU. It's highly recommended
  /// to always enable this setting.
  bool m_bOptimizeMesh{true};

  /// \brief The precision of the normal vectors.
  ezEnum<spAssimpVertexStreamComponentPrecision> m_eNormalPrecision{spAssimpVertexStreamComponentPrecision::Default};

  /// \brief The precision of the texture coordinates.
  ezEnum<spAssimpVertexStreamComponentPrecision> m_eTexCoordPrecision{spAssimpVertexStreamComponentPrecision::Default};

  /// \brief The precision of the bone weights.
  ezEnum<spAssimpVertexStreamComponentPrecision> m_eBoneWeightPrecision{spAssimpVertexStreamComponentPrecision::Default};
};

struct spAssimpBlendShape
{
  ezUInt32 m_uiParentMeshIndex{ezInvalidIndex};

  ezDynamicArray<ezUInt8, ezAlignedAllocatorWrapper> m_BlendShapeData;
  ezUInt32 m_uiVertexSize;
  float m_fWeight{1.0f};

  const aiAnimMesh* m_pAnimMesh{nullptr};
};

struct spAssimpMesh
{
  ezUInt32 m_uiParentNodeIndex{ezInvalidIndex};

  ezHashedString m_sName;
  ezDynamicArray<ezUInt32> m_MeshOptRemap;

  ezUInt32 m_uiBaseIndex{0};
  ezUInt32 m_uiBaseVertex{0};
  ezUInt32 m_uiIndexCount{0};
  ezUInt32 m_uiVertexCount{0};

  const aiMesh* m_pMesh{nullptr};
};

struct spAssimpNode
{
  ezUInt32 m_uiParentNodeIndex{ezInvalidIndex};
  ezMat4 m_mGlobalTransform;
  ezUInt32 m_uiLOD{0};

  const aiNode* m_pNode{nullptr};
};

/// \brief Assimp importer context. Stores the Assimp scene and parsed nodes.
struct spAssimpImporterContext
{
  ezUInt8 m_uiLODCount{0};
  ezUInt32 m_uiVertexCount{0};
  ezUInt32 m_uiPrimitiveCount{0};

  ezHashTable<ezHashedString, ezUInt32> m_MeshesNames;

  RAI::spMeshDataBuilder m_MeshDataBuilder;
  RAI::spMesh::Data m_MeshData;

  ezDynamicArray<spAssimpNode> m_Nodes;

  ezDynamicArray<spAssimpMesh> m_Meshes;
  ezDynamicArray<spAssimpBlendShape> m_BlendShapes;

  struct
  {
    ezUInt32 m_uiPositionStreamIndex{ezInvalidIndex};
    ezUInt32 m_uiNormalStreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord0StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord1StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord2StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord3StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord4StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord5StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord6StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTexCoord7StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor0StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor1StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor2StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor3StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor4StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor5StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor6StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiColor7StreamIndex{ezInvalidIndex};
    ezUInt32 m_uiTangentStreamIndex{ezInvalidIndex};
    ezUInt32 m_uiBiTangentStreamIndex{ezInvalidIndex};
    ezUInt32 m_uiBoneWeightsStreamIndex{ezInvalidIndex};
    ezUInt32 m_uiBoneIndicesStreamIndex{ezInvalidIndex};
  } m_Streams;

  const aiScene* m_pScene{nullptr};

  void Clear();
};

/// \brief Base class for assets importers using Assimp.
class spAssimpImporter : public spImporter<spAssimpImporterConfiguration>
{
  // spAssimpImporter

public:
  /// \brief Initializes a new instance of the importer.
  /// \param [in] configuration The importer configuration.
  explicit spAssimpImporter(const spAssimpImporterConfiguration& configuration);

  /// \brief Sets the context for the importer.
  /// \param [in] pContext The context.
  EZ_ALWAYS_INLINE void SetContext(const spAssimpImporterContext* pContext) { m_pContext = pContext; }

protected:
  const spAssimpImporterContext* m_pContext{nullptr};
};

#pragma region Assimp Data Conversion

static ezColor spFromAssimp(const aiColor4D& value, bool bInvert = false)
{
  if (bInvert)
    return {1.0f - value.r, 1.0f - value.g, 1.0f - value.b, 1.0f - value.a};

  return {value.r, value.g, value.b, value.a};
}

static ezColor spFromAssimp(const aiColor3D& value, bool bInvert = false)
{
  if (bInvert)
    return {1.0f - value.r, 1.0f - value.g, 1.0f - value.b};

  return {value.r, value.g, value.b};
}

static ezMat4 spFromAssimp(const aiMatrix4x4& value)
{
  ezMat4 mTransformation = ezMat4::MakeFromRowMajorArray(&value.a1);
  return mTransformation;
}

static ezVec3 spFromAssimp(const aiVector3D& value)
{
  return {value.x, value.y, value.z};
}

static ezQuat spFromAssimp(const aiQuaternion& value)
{
  return {value.x, value.y, value.z, value.w};
}

static float spFromAssimp(float value)
{
  return value;
}

static int spFromAssimp(int value)
{
  return value;
}

#pragma endregion
