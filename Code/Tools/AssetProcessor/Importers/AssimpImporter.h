#pragma once

#include <AssetProcessor/Importers/Importer.h>

#include <RAI/Mesh.h>

#include <Foundation/Strings/HashedString.h>

#include <assimp/Importer.hpp>

struct aiAnimMesh;
struct aiMesh;
struct aiNode;

/// \brief Assimp Importer settings.
struct spAssimpImporterConfiguration
{
  /// \brief The global scale to apply on the imported mesh.
  float m_fScale{1.0f};

  /// \brief Specifies if the mesh was exported from a DCC tool using the Spark LOD system specifications.
  bool m_bHasLODs{true};

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
};

struct spAssimpBlendShape
{
  ezUInt32 m_uiParentMeshIndex{ezInvalidIndex};

  ezDynamicArray<RAI::spVertex> m_BlendShapeData;
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

  ezUInt8 m_uiLODLevel{0};

  const aiNode* m_pNode{nullptr};
};

/// \brief Assimp importer context. Stores the Assimp scene and parsed nodes.
struct spAssimpImporterContext
{
  ezUInt8 m_uiLODCount{0};

  ezHashTable<ezHashedString, ezUInt32> m_MeshesNames;

  ezDynamicArray<spAssimpNode> m_Nodes;

  RAI::spMesh::Data m_MeshData;
  ezDynamicArray<spAssimpMesh> m_Meshes;
  ezDynamicArray<spAssimpBlendShape> m_BlendShapes;

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
  ezMat4 mTransformation;
  mTransformation.SetFromArray(&value.a1, ezMatrixLayout::RowMajor);
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
