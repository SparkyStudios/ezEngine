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

#include <AssetProcessor/Importers/MeshImporter.h>

#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#include <assimp/scene.h>

using namespace RAI;

static void ExtractTransform(const aiNode* pNode, spMesh::Node& ref_node)
{
  aiVector3D s, r, p;
  pNode->mTransformation.Decompose(s, r, p);

  ref_node.m_Transform = {spFromAssimp(p), spFromAssimp(s), spFromAssimp(r)};
}

static void ExtractMaterial(const aiNode* pNode, spMesh::Node& ref_node)
{
  // Allow artists to give the name of the material asset used by this node directly from the DCC tool
  aiString sMaterialName;
  if (pNode->mMetaData != nullptr && pNode->mMetaData->Get<aiString>("spark.mesh.material", sMaterialName))
    ref_node.m_sMaterial.Assign(sMaterialName.C_Str());
}

ezResult spMeshImporter::Import(ezStringView sFilePath, ezStringView sOutputPath)
{
  if (m_pContext == nullptr)
  {
    ezLog::Error("Invalid Assimp Context.");
    return EZ_FAILURE;
  }

  spMeshResourceDescriptor mesh;

  if (m_Configuration.m_bHasLODs)
  {
    ezUInt32 uiVerticesBaseIndex = 0, uiIndicesBaseIndex = 0;

    for (ezUInt8 i = 0; i < m_pContext->m_uiLODCount; ++i)
    {
      spMesh m;
      if (ImportLODMeshes(i, &m, uiVerticesBaseIndex, uiIndicesBaseIndex).Failed())
        return EZ_FAILURE;

      m.ComputeBounds();

      mesh.SetLOD(i, m);
    }
  }
  else
  {
    const ezUInt32 uiMeshCount = m_pContext->m_Meshes.GetCount();

    ezDynamicArray<spMesh::Entry> entries;
    entries.SetCount(uiMeshCount);

    for (ezUInt32 uiMeshIndex = 0; uiMeshIndex < uiMeshCount; ++uiMeshIndex)
    {
      const spAssimpMesh& assimpMesh = m_pContext->m_Meshes[uiMeshIndex];

      entries[uiMeshIndex].m_sName = assimpMesh.m_sName;
      entries[uiMeshIndex].m_uiBaseIndex = assimpMesh.m_uiBaseIndex;
      entries[uiMeshIndex].m_uiBaseVertex = assimpMesh.m_uiBaseVertex;
      entries[uiMeshIndex].m_uiIndexCount = assimpMesh.m_uiIndexCount;
      entries[uiMeshIndex].m_uiVertexCount = assimpMesh.m_uiVertexCount;
    }

    spMesh::Node root;

    aiVector3D s, r, p;
    m_pContext->m_pScene->mRootNode->mTransformation.Decompose(s, r, p);

    root.m_sName.Assign(m_pContext->m_pScene->mRootNode->mName.C_Str());
    root.m_Transform = {spFromAssimp(p), spFromAssimp(s), spFromAssimp(r)};

    ComputeMeshHierarchy(entries, 0, ezInvalidIndex, root);

    spMesh lod;
    lod.SetData(m_pContext->m_MeshData);
    lod.SetRootNode(root);
    lod.ComputeBounds();

    mesh.SetLOD(0, lod);
  }

  ezStringBuilder sOutputFile(sOutputPath);
  sOutputFile.AppendFormat("/{}.spMesh", sFilePath.GetFileName());

  ezFileWriter file;
  if (file.Open(sOutputFile, 1024 * 1024).Failed())
  {
    ezLog::Error("Failed to save mesh asset: '{0}'", sOutputFile);
    return EZ_FAILURE;
  }

  // Write asset header
  ezAssetFileHeader assetHeader;
  assetHeader.SetGenerator("SparkEngine Asset Processor");
  assetHeader.SetFileHashAndVersion(ezHashingUtils::xxHash64String(sFilePath), 1);
  EZ_SUCCEED_OR_RETURN(assetHeader.Write(file));

  EZ_SUCCEED_OR_RETURN(mesh.Save(file));

  return EZ_SUCCESS;
}

spMeshImporter::spMeshImporter(const spAssimpImporterConfiguration& configuration)
  : spAssimpImporter(configuration)
{
}

void spMeshImporter::CollectLODMeshes(ezUInt32 uiNodeIndex, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex, ezDynamicArray<spMesh::Entry>& ref_entries, ezUInt32& ref_uiVerticesUpperBound, ezUInt32& ref_uiIndicesUpperBound)
{
  for (ezUInt32 uiMeshIndex = 0, l = m_pContext->m_Meshes.GetCount(); uiMeshIndex < l; ++uiMeshIndex)
  {
    const spAssimpMesh& assimpMesh = m_pContext->m_Meshes[uiMeshIndex];
    if (assimpMesh.m_uiParentNodeIndex != uiNodeIndex)
      continue;

    auto& entry = ref_entries[uiMeshIndex];
    entry.m_sName = assimpMesh.m_sName;
    entry.m_uiBaseIndex = assimpMesh.m_uiBaseIndex - out_uiBaseIndex;
    entry.m_uiBaseVertex = assimpMesh.m_uiBaseVertex - out_uiBaseVertex;
    entry.m_uiIndexCount = assimpMesh.m_uiIndexCount;
    entry.m_uiVertexCount = assimpMesh.m_uiVertexCount;

    out_uiBaseVertex = ezMath::Min(out_uiBaseVertex, assimpMesh.m_uiBaseVertex);
    ref_uiVerticesUpperBound = ezMath::Max(ref_uiVerticesUpperBound, assimpMesh.m_uiBaseVertex + assimpMesh.m_uiVertexCount);

    out_uiBaseIndex = ezMath::Min(out_uiBaseIndex, assimpMesh.m_uiBaseIndex);
    ref_uiIndicesUpperBound = ezMath::Max(ref_uiIndicesUpperBound, assimpMesh.m_uiBaseIndex + assimpMesh.m_uiIndexCount);

    for (ezUInt32 i = 0, m = m_pContext->m_Nodes[uiNodeIndex].m_pNode->mNumChildren; i < m; ++i)
      CollectLODMeshes(i, out_uiBaseVertex, out_uiBaseIndex, ref_entries, ref_uiVerticesUpperBound, ref_uiIndicesUpperBound);
  }
}

ezResult spMeshImporter::ImportLODMeshes(ezUInt8 uiLOD, spMesh* out_pMesh, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex)
{
  const ezUInt32 uiMeshCount = m_pContext->m_Meshes.GetCount();

  ezDynamicArray<spMesh::Entry> entries;
  entries.SetCount(uiMeshCount);

  ezUInt32 uiRootNode = ezInvalidIndex;

  for (ezUInt32 i = 0, l = m_pContext->m_Nodes.GetCount(); i < l; ++i)
  {
    if (m_pContext->m_Nodes[i].m_uiLOD == uiLOD)
    {
      uiRootNode = i;
      break;
    }
  }

  if (uiRootNode == ezInvalidIndex)
  {
    ezLog::Error("Could not find root node for LOD '{0}'. Please insure that the mesh has been exported following the Spark LOD specifications.", uiLOD);
    return EZ_FAILURE;
  }

  ezUInt32 uiVerticesUpperBound = 0;
  ezUInt32 uiIndicesUpperBound = 0;

  CollectLODMeshes(uiRootNode, out_uiBaseVertex, out_uiBaseIndex, entries, uiVerticesUpperBound, uiIndicesUpperBound);

  spMesh::Data meshData;
  meshData.m_VertexStreams.PushBackRange(m_pContext->m_MeshData.m_VertexStreams);

  meshData.m_Vertices.SetCount((uiVerticesUpperBound - out_uiBaseVertex) * m_pContext->m_MeshData.m_uiVertexSize);
  ezMemoryUtils::RawByteCopy(meshData.m_Vertices.GetData(), m_pContext->m_MeshData.m_Vertices.GetData() + out_uiBaseVertex * m_pContext->m_MeshData.m_uiVertexSize, meshData.m_Vertices.GetCount());

  meshData.m_Indices.SetCount((uiIndicesUpperBound - out_uiBaseIndex) * sizeof(ezUInt16));
  ezMemoryUtils::RawByteCopy(meshData.m_Indices.GetData(), m_pContext->m_MeshData.m_Indices.GetData() + out_uiBaseIndex * sizeof(ezUInt16), meshData.m_Indices.GetCount());

  meshData.m_uiVertexSize = m_pContext->m_MeshData.m_uiVertexSize;
  meshData.m_eTopology = m_pContext->m_MeshData.m_eTopology;

  out_uiBaseIndex = uiIndicesUpperBound;
  out_uiBaseVertex = uiVerticesUpperBound;

  spMesh::Node root;
  const aiNode* pNode = m_pContext->m_Nodes[uiRootNode].m_pNode;

  aiVector3D s, r, p;
  pNode->mTransformation.Decompose(s, r, p);

  root.m_sName.Assign(pNode->mName.C_Str());
  root.m_Transform = {spFromAssimp(p), spFromAssimp(s), spFromAssimp(r)};

  ComputeMeshHierarchy(entries, 0, uiLOD, root);

  out_pMesh->SetData(meshData);
  out_pMesh->SetRootNode(root);

  return EZ_SUCCESS;
}

void spMeshImporter::ComputeMeshHierarchy(const ezDynamicArray<spMesh::Entry>& nodeEntries, ezUInt32 uiParentIndex, ezUInt32 uiLOD, spMesh::Node& ref_root)
{
  for (ezUInt32 i = 0, l = m_pContext->m_Nodes.GetCount(); i < l; ++i)
  {
    const spAssimpNode& assimpNode = m_pContext->m_Nodes[i];

    if (assimpNode.m_uiParentNodeIndex != uiParentIndex || assimpNode.m_uiLOD != uiLOD)
      continue;

    spMesh::Node node;
    const aiNode* pNode = assimpNode.m_pNode;

    node.m_sName.Assign(pNode->mName.C_Str());

    ExtractTransform(pNode, node);

    for (ezUInt32 m = 0, k = m_pContext->m_Meshes.GetCount(); m < k; ++m)
      if (m_pContext->m_Meshes[m].m_uiParentNodeIndex == i)
        node.m_Entries.ExpandAndGetRef() = nodeEntries[m];

    // Add children if any
    if (pNode->mNumChildren > 0)
      ComputeMeshHierarchy(nodeEntries, i, ezInvalidIndex, node);

    ExtractMaterial(pNode, node);

    ref_root.m_Children.PushBack(std::move(node));
  }
}
