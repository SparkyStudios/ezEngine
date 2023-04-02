#pragma once

#include <AssetProcessor/Importers/AssimpImporter.h>

#include <RAI/Resources/MeshResource.h>

/// \brief spAssimpImporter implementation for meshes.
class spMeshImporter : public spAssimpImporter
{
  // spAssimpImporter

public:
  ezResult Import(ezStringView sFilePath, ezStringView sOutputPath) override;

  // spMeshImporter

public:
  explicit spMeshImporter(const spAssimpImporterConfiguration& configuration);

private:
  void ImportLODMeshes(ezUInt8 uiLODLevel, RAI::spMesh* out_pMesh, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex);
  void ComputeMeshHierarchy(const ezDynamicArray<RAI::spMesh::Entry>& nodeEntries, ezUInt32 uiParentIndex, RAI::spMesh::Node& ref_root);
};
