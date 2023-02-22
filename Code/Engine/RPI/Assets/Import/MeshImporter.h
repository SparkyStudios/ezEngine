#pragma once

#include <RPI/RPIDLL.h>

#include <RPI/Assets/Import/Importer.h>
#include <RPI/Assets/Mesh.h>

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

class SP_RPI_DLL spMeshImporter final : public spImporter<spMeshImporterConfiguration, spMesh>
{
  // spImporter<spMeshImporterConfiguration, spMesh>

public:
  ezResult Import(ezStringView sSourceFile, spMesh* out_pAsset) override;

  // spMeshImporter

public:
  explicit spMeshImporter(const spMeshImporterConfiguration& configuration);
  ~spMeshImporter() override;

private:
  void ImportMeshes(const aiScene* pScene, spMesh* out_pMesh);

  spMesh::Node ComputeMeshHierarchy(const ezDynamicArray<spMesh::Entry>& allEntries, const aiNode* pRootNode);
  void ImportMesh(spMesh::Data& data, const aiMesh* pMesh);

  ezStringView m_sFilePath;

  Assimp::Importer m_Importer;
};
