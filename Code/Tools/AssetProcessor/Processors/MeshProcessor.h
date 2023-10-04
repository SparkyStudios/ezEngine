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
#include <AssetProcessor/Importers/BlendShapeImporter.h>
#include <AssetProcessor/Importers/MeshImporter.h>
#include <AssetProcessor/Importers/SkeletonImporter.h>
#include <AssetProcessor/Processors/Processor.h>

#include <assimp/Importer.hpp>

/// \brief Describes the required configuration for the mesh asset processor.
struct spMeshProcessorConfig
{
  /// \brief Whether to import skeletons in the process.
  bool m_bImportSkeleton{false};

  /// \brief Whether to import animations in the process.
  bool m_bImportMotions{false};

  /// \brief Whether to import materials in the process.
  bool m_bImportMaterials{false};

  /// \brief Whether to import meshes in the process.
  bool m_bImportMeshes{true};

  spAssimpImporterConfiguration m_AssimpImporterConfig;
};

/// \brief Mesh files processor.
///
/// This file processor can generate mesh, skeleton, motion and blend shape
/// assets from 3D mesh files using Assimp.
class spMeshProcessor final : public spProcessor<spMeshProcessorConfig>
{
  // spProcessor

public:
  ezResult Process(ezStringView sFilename, ezStringView sOutputPath) override;

  // spMeshProcessor

public:
  explicit spMeshProcessor(const spMeshProcessorConfig& config);
  ~spMeshProcessor() override = default;

private:
  ezResult BuildContext();

  void ProcessNodes(const aiNode* pNode, ezUInt32 uiParentIndex, ezUInt8& inout_uiLODCount);

  void ProcessMeshes(const aiNode* pNode, ezUInt32 uiParentIndex, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex);
  void ProcessBlendShapes();

  void OptimizeMeshData(float* pVertexBuffer, ezUInt32 uiVertexCount, ezUInt16* pIndexBuffer, ezUInt32 uiIndexCount, ezDynamicArray<ezUInt32>& ref_remap) const;

  spBlendShapeImporter m_BlendShapeImporter;
  spMeshImporter m_MeshImporter;
  spSkeletonImporter m_SkeletonImporter;

  Assimp::Importer m_Importer;
  const aiScene* m_pScene{nullptr};

  spAssimpImporterContext m_AssimpContext;
};
