#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Import/Importer.h>
#include <RAI/Mesh.h>

#include <assimp/Importer.hpp>

class aiMesh;
class aiNode;

struct spMeshImporterConfiguration
{
  float m_fScale{1.0f};
  bool m_bImportSkeleton{false};
  bool m_bImportAnimations{false};
  bool m_bImportMaterials{false};
  bool m_bImportMeshes{true};
  bool m_bFlipUVs{false};
  bool m_bRecomputeNormals{false};
  bool m_bRecomputeTangents{false};
  bool m_bFlipWindingNormals{false};
  bool m_bOptimizeMesh{true};
};

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
  void ImportLODs(const aiScene* pScene, spMesh* out_pMesh, ezUInt32 uiCount);
  void ImportMeshes(const aiScene* pScene, const aiNode* pNode, spMesh* out_pMesh);
  spMesh::Node ComputeMeshHierarchy(const ezDynamicArray<spMesh::Entry>& allEntries, const aiNode* pNode);
  void ImportMesh(spMesh::Data& data, const aiMesh* pMesh);
  void RecomputeTangents(spMesh::Data& data);

  ezStringView m_sFilePath;

  Assimp::Importer m_Importer;
};
