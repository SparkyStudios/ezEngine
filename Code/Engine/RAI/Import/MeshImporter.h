#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Import/Importer.h>
#include <RAI/Mesh.h>

#include <assimp/Importer.hpp>

class aiMesh;
class aiNode;

namespace RAI
{
  /// \brief Mesh Importer settings.
  struct spMeshImporterConfiguration
  {
    /// \brief The global scale to apply on the imported mesh.
    float m_fScale{1.0f};

    /// \brief Whether to import skeletons in the process.
    bool m_bImportSkeleton{false};

    /// \brief Whether to import animations in the process.
    bool m_bImportAnimations{false};

    /// \brief Whether to import materials in the process.
    bool m_bImportMaterials{false};

    /// \brief Whether to import meshes in the process.
    bool m_bImportMeshes{true};

    /// \brief Whether to import the mesh using the Spark LOD system. Meshes should be exported
    /// following the specifications of the Spark LOD system.
    bool m_bImportLODs{true};

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

  /// \brief spImporter implementation for meshes.
  class SP_RAI_DLL spMeshImporter final : public spImporter<spMeshImporterConfiguration, spMesh>
  {
    // spImporter<spMeshImporterConfiguration, spMesh>

  public:
    ezResult Import(ezStringView sSourceFile, spMesh* out_pAsset, ezUInt32 uiCount) override;

    // spMeshImporter

  public:
    explicit spMeshImporter(const spMeshImporterConfiguration& configuration);
    ~spMeshImporter() override;

  private:
    void ImportWithLODs(const aiScene* pScene, spMesh* out_pMesh, ezUInt32 uiCount);
    void ImportWithoutLODs(const aiScene* pScene, spMesh* out_pMesh, ezUInt32 uiCount);
    void ImportMeshes(const aiScene* pScene, const aiNode* pNode, spMesh* out_pMesh);
    spMesh::Node ComputeMeshHierarchy(const ezDynamicArray<spMesh::Entry>& allEntries, const aiNode* pNode);
    void ImportMesh(spMesh::Data& data, const aiMesh* pMesh);
    void RecomputeTangents(spMesh::Data& data);

    ezStringView m_sFilePath;

    Assimp::Importer m_Importer;
  };
} // namespace RAI
