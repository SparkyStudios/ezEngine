#include <AssetProcessor/Importers/MeshImporter.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

#include <assimp/scene.h>

using namespace RAI;

static void ExtractTransform(const aiNode* pNode, spMesh::Node& node)
{
  aiVector3D s, r, p;
  pNode->mTransformation.Decompose(s, r, p);

  node.m_Transform = {spFromAssimp(p), spFromAssimp(s), spFromAssimp(r)};
}

static void ExtractMaterial(const aiNode* pNode, spMesh::Node& node)
{
  // Allow artists to give the name of the material asset used by this node directly from the DCC tool
  aiString sMaterialName;
  if (pNode->mMetaData != nullptr && pNode->mMetaData->Get<aiString>("spark.mesh.material", sMaterialName))
    node.m_sMaterial.Assign(sMaterialName.C_Str());
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
    ezUInt32 uiVerticesLowerBound = 0, uiVerticesUpperBound = 0;
    ezUInt32 uiIndicesLowerBound = 0, uiIndicesUpperBound = 0;

    for (ezUInt8 i = 0; i < m_pContext->m_uiLODCount; ++i)
    {
      spMesh m;
      ImportLODMeshes(i, &m, uiVerticesLowerBound, uiIndicesUpperBound);

      mesh.SetLOD(m_pContext->m_Nodes[i].m_uiLODLevel, m);
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

      entries[uiMeshIndex].m_sName.Assign(assimpMesh.m_pMesh->mName.C_Str());
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

    ComputeMeshHierarchy(entries, ezInvalidIndex, root);

    mesh.SetLOD(0, {m_pContext->m_MeshData, root});
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

void spMeshImporter::ImportLODMeshes(ezUInt8 uiLODLevel, spMesh* out_pMesh, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex)
{
  const ezUInt32 uiMeshCount = m_pContext->m_Meshes.GetCount();

  ezDynamicArray<spMesh::Entry> entries;
  entries.SetCount(uiMeshCount);

  ezDynamicArray<ezUInt32> lodNodes;
  lodNodes.PushBack(uiLODLevel);

  for (ezUInt32 i = 0, l = m_pContext->m_Nodes.GetCount(); i < l; ++i)
    if (m_pContext->m_Nodes[i].m_uiParentNodeIndex == uiLODLevel)
      lodNodes.PushBack(i);

  ezUInt32 uiVerticesUpperBound = 0;
  ezUInt32 uiIndicesUpperBound = 0;

  for (ezUInt32 uiMeshIndex = 0; uiMeshIndex < uiMeshCount; ++uiMeshIndex)
  {
    const spAssimpMesh& assimpMesh = m_pContext->m_Meshes[uiMeshIndex];
    if (lodNodes.IndexOf(assimpMesh.m_uiParentNodeIndex) == ezInvalidIndex)
      continue;

    entries[uiMeshIndex].m_sName.Assign(assimpMesh.m_pMesh->mName.C_Str());
    entries[uiMeshIndex].m_uiBaseIndex = assimpMesh.m_uiBaseIndex - out_uiBaseIndex;
    entries[uiMeshIndex].m_uiBaseVertex = assimpMesh.m_uiBaseVertex - out_uiBaseVertex;
    entries[uiMeshIndex].m_uiIndexCount = assimpMesh.m_uiIndexCount;
    entries[uiMeshIndex].m_uiVertexCount = assimpMesh.m_uiVertexCount;

    out_uiBaseVertex = ezMath::Min(out_uiBaseVertex, assimpMesh.m_uiBaseVertex);
    uiVerticesUpperBound = ezMath::Max(uiVerticesUpperBound, assimpMesh.m_uiBaseVertex + assimpMesh.m_uiVertexCount);

    out_uiBaseIndex = ezMath::Min(out_uiBaseIndex, assimpMesh.m_uiBaseIndex);
    uiIndicesUpperBound = ezMath::Max(uiIndicesUpperBound, assimpMesh.m_uiBaseIndex + assimpMesh.m_uiIndexCount);
  }

  spMesh::Data meshData;
  meshData.m_Vertices.PushBackRange(m_pContext->m_MeshData.m_Vertices.GetArrayPtr().GetSubArray(out_uiBaseVertex, uiVerticesUpperBound - out_uiBaseVertex));
  meshData.m_Indices.PushBackRange(m_pContext->m_MeshData.m_Indices.GetArrayPtr().GetSubArray(out_uiBaseIndex, uiIndicesUpperBound - out_uiBaseIndex));

  out_uiBaseIndex = uiIndicesUpperBound;
  out_uiBaseVertex = uiVerticesUpperBound;

  spMesh::Node root;
  const aiNode* pNode = m_pContext->m_Nodes[uiLODLevel].m_pNode;

  ExtractTransform(pNode, root);

  root.m_sName.Assign(pNode->mName.C_Str());

  for (ezUInt32 m = 0, k = m_pContext->m_Meshes.GetCount(); m < k; ++m)
  {
    if (m_pContext->m_Meshes[m].m_uiParentNodeIndex == uiLODLevel)
      root.m_Entries.ExpandAndGetRef() = entries[m];
  }

  ExtractMaterial(pNode, root);

  ComputeMeshHierarchy(entries, uiLODLevel, root);

  out_pMesh->SetData(meshData);
  out_pMesh->SetRootNode(root);
}

void spMeshImporter::ComputeMeshHierarchy(const ezDynamicArray<spMesh::Entry>& nodeEntries, ezUInt32 uiParentIndex, spMesh::Node& ref_root)
{
  for (ezUInt32 i = 0, l = m_pContext->m_Nodes.GetCount(); i < l; ++i)
  {
    const spAssimpNode& assimpNode = m_pContext->m_Nodes[i];

    if (assimpNode.m_uiParentNodeIndex == uiParentIndex)
    {
      spMesh::Node node;

      const aiNode* pNode = assimpNode.m_pNode;

      node.m_sName.Assign(pNode->mName.C_Str());

      ExtractTransform(pNode, node);

      for (ezUInt32 m = 0, k = m_pContext->m_Meshes.GetCount(); m < k; ++m)
      {
        if (m_pContext->m_Meshes[m].m_uiParentNodeIndex == i)
          node.m_Entries.ExpandAndGetRef() = nodeEntries[m];
      }

      // Add children if any
      if (pNode->mNumChildren > 0)
        ComputeMeshHierarchy(nodeEntries, i, node);

      ExtractMaterial(pNode, node);

      ref_root.m_Children.PushBack(std::move(node));
    }
  }
}
