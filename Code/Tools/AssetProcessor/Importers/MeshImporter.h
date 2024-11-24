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

#include <AssetProcessor/Importers/AssimpImporter.h>

#include <RPI/Resources/MeshResource.h>

/// \brief spAssimpImporter implementation for meshes.
class spMeshImporter final : public spAssimpImporter
{
  // spAssimpImporter

public:
  ezResult Import(ezStringView sFilePath, ezStringView sOutputPath) override;

  // spMeshImporter

public:
  explicit spMeshImporter(const spAssimpImporterConfiguration& configuration);

private:
  void CollectLODMeshes(ezUInt32 uiNodeIndex, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex, ezDynamicArray<RPI::spMesh::Entry>& ref_entries, ezUInt32& ref_uiVerticesUpperBound, ezUInt32& ref_uiIndicesUpperBound);
  ezResult ImportLODMeshes(ezUInt8 uiLOD, RPI::spMesh* out_pMesh, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex);
  void ComputeMeshHierarchy(const ezDynamicArray<RPI::spMesh::Entry>& nodeEntries, ezUInt32 uiParentIndex, ezUInt32 uiLOD, RPI::spMesh::Node& ref_root);
};
