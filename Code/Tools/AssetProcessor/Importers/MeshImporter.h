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
