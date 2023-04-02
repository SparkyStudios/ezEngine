#include <AssetProcessor/Processors/MeshProcessor.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meshoptimizer.h>

#include <mikktspace/mikktspace.h>

using namespace RAI;


static constexpr float kMeshOptimizationThreshold = 1.01f; // allow up to 1% worse ACMR to get more reordering opportunities for overdraw

#pragma region MikkT Tangent Space Generation

struct spMikkTSpaceContext
{
  const ezUInt16* m_pIndices{nullptr};
  ezUInt32 m_uiNumIndices{0};

  spVertex* m_pVertices{nullptr};
  ezUInt32 m_uiNumVertices{0};
};

static int MikkTGetNumFaces(const SMikkTSpaceContext* pContext)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData);

  const auto fSize = static_cast<float>(pMeshData->m_uiNumIndices) / 3.0f;
  const auto iSize = static_cast<int>(pMeshData->m_uiNumIndices) / 3;

  EZ_ASSERT_DEV(fSize - static_cast<float>(iSize) == 0.0f, "There are faces in the mesh that are not triangles.");

  return iSize;
}

static int MikkTGetNumVerticesOfFace(const SMikkTSpaceContext* context, int iFace)
{
  // We only work with triangles
  return 3;
}

static int MikkTGetVertexIndex(const SMikkTSpaceContext* pContext, int iFace, int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData);

  const int iFaceSize = MikkTGetNumVerticesOfFace(pContext, iFace);
  const int iIndicesIndex = iFace * iFaceSize + iVert;

  return pMeshData->m_pIndices[iIndicesIndex];
}

static void MikkTGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData);

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const spVertex vertex = pMeshData->m_pVertices[iIndex];

  fvNormOut[0] = vertex.m_vNormal.x;
  fvNormOut[1] = vertex.m_vNormal.y;
  fvNormOut[2] = vertex.m_vNormal.z;
}

static void MikkTGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData);

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const spVertex vertex = pMeshData->m_pVertices[iIndex];

  fvPosOut[0] = vertex.m_vPosition.x;
  fvPosOut[1] = vertex.m_vPosition.y;
  fvPosOut[2] = vertex.m_vPosition.z;
}

static void MikkTGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData);

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const spVertex vertex = pMeshData->m_pVertices[iIndex];

  fvTexcOut[0] = vertex.m_vTexCoord0.x;
  fvTexcOut[1] = vertex.m_vTexCoord0.y;
}

static void MikkTSetTSpace(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT, const tbool bIsOrientationPreserving, const int iFace, const int iVert)
{
  auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData);

  const auto index = MikkTGetVertexIndex(pContext, iFace, iVert);
  auto* vertex = &pMeshData->m_pVertices[index];

  vertex->m_vTangent.x = fvTangent[0];
  vertex->m_vTangent.y = fvTangent[1];
  vertex->m_vTangent.z = fvTangent[2];
  vertex->m_vTangent.w = fMagS;

  vertex->m_vBiTangent.x = fvBiTangent[0];
  vertex->m_vBiTangent.y = fvBiTangent[1];
  vertex->m_vBiTangent.z = fvBiTangent[2];
  vertex->m_vBiTangent.w = fMagT;
}

static void RecomputeTangents(const ezArrayPtr<spVertex>& vertices, const ezArrayPtr<ezUInt16>& indices)
{
  SMikkTSpaceInterface mkt;
  SMikkTSpaceContext mktContext;

  mkt.m_getNumFaces = MikkTGetNumFaces;
  mkt.m_getNumVerticesOfFace = MikkTGetNumVerticesOfFace;
  mkt.m_getNormal = MikkTGetNormal;
  mkt.m_getPosition = MikkTGetPosition;
  mkt.m_getTexCoord = MikkTGetTexCoord;
  mkt.m_setTSpace = MikkTSetTSpace;
  mkt.m_setTSpaceBasic = nullptr;

  spMikkTSpaceContext context;
  context.m_pIndices = indices.GetPtr();
  context.m_uiNumIndices = indices.GetCount();
  context.m_pVertices = vertices.GetPtr();
  context.m_uiNumVertices = vertices.GetCount();

  mktContext.m_pInterface = &mkt;
  mktContext.m_pUserData = &context;

  genTangSpaceDefault(&mktContext);
}

#pragma endregion

#pragma region Assimp Logging

class aiLogStreamError final : public Assimp::LogStream
{
public:
  void write(const char* message) override { ezLog::Warning("AssImp: {0}", message); }
};

class aiLogStreamWarning final : public Assimp::LogStream
{
public:
  void write(const char* message) override { ezLog::Warning("AssImp: {0}", message); }
};

class aiLogStreamInfo final : public Assimp::LogStream
{
public:
  void write(const char* message) override { ezLog::Dev("AssImp: {0}", message); }
};

#pragma endregion

ezResult spMeshProcessor::Process(ezStringView sFilename, ezStringView sOutputPath)
{
  EZ_LOG_BLOCK("spMeshProcessor::Process", sFilename);

  Assimp::DefaultLogger::create("", Assimp::Logger::NORMAL);

  Assimp::DefaultLogger::get()->attachStream(new aiLogStreamError(), Assimp::Logger::Err);
  Assimp::DefaultLogger::get()->attachStream(new aiLogStreamWarning(), Assimp::Logger::Warn);
  Assimp::DefaultLogger::get()->attachStream(new aiLogStreamInfo(), Assimp::Logger::Info);

  m_Importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, m_Configuration.m_AssimpImporterConfig.m_fScale);
  m_Importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, UINT16_MAX);
  m_Importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

  m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
  m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, false);

  m_sFilePath = sFilename;
  m_sOutputDir = sOutputPath;

  ezUInt32 uiPreset = aiProcess_SplitLargeMeshes | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FindDegenerates | aiProcess_FindInstances | aiProcess_FindInvalidData | aiProcess_RemoveComponent | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_GlobalScale;

  if (m_Configuration.m_AssimpImporterConfig.m_bFlipWindingNormals)
    uiPreset |= aiProcess_FixInfacingNormals;
  if (m_Configuration.m_AssimpImporterConfig.m_bFlipUVs)
    uiPreset |= aiProcess_FlipUVs;
  if (m_Configuration.m_AssimpImporterConfig.m_bRecomputeNormals)
    uiPreset |= aiProcess_ForceGenNormals;

  if (m_Configuration.m_bImportSkeleton)
    uiPreset |= aiProcess_PopulateArmatureData;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  uiPreset |= aiProcess_ValidateDataStructure;
#endif

  ezStringBuilder sb;
  m_pScene = m_Importer.ReadFile(sFilename.GetData(sb), uiPreset);
  if (m_pScene == nullptr)
  {
    ezLog::Error("Assimp failed to import '{}'", sFilename);
    return EZ_FAILURE;
  }

  EZ_SUCCEED_OR_RETURN(BuildContext());

  // Import meshes
  if (m_Configuration.m_bImportMeshes)
  {
    m_MeshImporter.SetContext(&m_AssimpContext);
    EZ_SUCCEED_OR_RETURN(m_MeshImporter.Import(sFilename, sOutputPath));

    // Import motions
    if (m_Configuration.m_bImportMotions)
    {
      m_BlendShapeImporter.SetContext(&m_AssimpContext);
      EZ_SUCCEED_OR_RETURN(m_BlendShapeImporter.Import(sFilename, sOutputPath));
    }
  }

  // Import skeleton
  if (m_Configuration.m_bImportSkeleton)
  {
    m_SkeletonImporter.SetContext(&m_AssimpContext);
    EZ_SUCCEED_OR_RETURN(m_SkeletonImporter.Import(sFilename, sOutputPath));
  }

  return EZ_SUCCESS;
}

spMeshProcessor::spMeshProcessor(const spMeshProcessorConfig& config)
  : spProcessor(config)
  , m_BlendShapeImporter(config.m_AssimpImporterConfig)
  , m_MeshImporter(config.m_AssimpImporterConfig)
  , m_SkeletonImporter(config.m_AssimpImporterConfig)
{
}

ezResult spMeshProcessor::BuildContext()
{
  m_AssimpContext.Clear();

  // Set Assimp scene
  m_AssimpContext.m_pScene = m_pScene;

  // Process nodes
  {
    ezUInt8 uiLODCount = 0;

    for (ezUInt32 n = 0; n < m_pScene->mRootNode->mNumChildren; ++n)
    {
      const aiNode* pNode = m_pScene->mRootNode->mChildren[n];
      ProcessNodes(pNode, ezInvalidIndex, uiLODCount);
    }

    m_AssimpContext.m_uiLODCount = uiLODCount + 1;
  }

  // Process mesh data
  if (m_Configuration.m_bImportMeshes)
  {
    ezUInt32 uiBaseVertex = 0;
    ezUInt32 uiBaseIndex = 0;

    // Read mesh nodes
    for (ezUInt32 n = 0, l = m_AssimpContext.m_Nodes.GetCount(); n < l; ++n)
    {
      ProcessMeshes(m_AssimpContext.m_Nodes[n].m_pNode, n, uiBaseVertex, uiBaseIndex);
    }

    // Ensure no memory is wasted
    m_AssimpContext.m_MeshData.m_Indices.SetCount(uiBaseIndex);
    m_AssimpContext.m_MeshData.m_Vertices.SetCount(uiBaseVertex);

    m_AssimpContext.m_MeshData.m_Indices.Compact();
    m_AssimpContext.m_MeshData.m_Vertices.Compact();

    // Recompute tangent space if required
    if (m_Configuration.m_AssimpImporterConfig.m_bRecomputeTangents)
      RecomputeTangents(m_AssimpContext.m_MeshData.m_Vertices, m_AssimpContext.m_MeshData.m_Indices);

    // Process blend shape data
    if (m_Configuration.m_bImportMotions)
    {
      ProcessBlendShapes();
    }
  }

  return EZ_SUCCESS;
}

void spMeshProcessor::ProcessNodes(const aiNode* pNode, ezUInt32 uiParentIndex, ezUInt8& inout_uiLODCount)
{
  bool bShouldSkip = false;
  if (pNode->mMetaData != nullptr && pNode->mMetaData->Get<bool>("spark.mesh.import.skip", bShouldSkip) && bShouldSkip)
    return;

  ezInt32 uiLODLevel = 0;
  if (pNode->mMetaData != nullptr)
    pNode->mMetaData->Get<ezInt32>("spark.mesh.lod", uiLODLevel);

  spAssimpNode& node = m_AssimpContext.m_Nodes.ExpandAndGetRef();

  node.m_pNode = pNode;
  node.m_uiParentNodeIndex = uiParentIndex;
  node.m_uiLODLevel = static_cast<ezUInt8>(uiLODLevel);

  if (node.m_uiLODLevel > inout_uiLODCount)
    inout_uiLODCount = node.m_uiLODLevel;

  const ezUInt32 uiNodeIndex = m_AssimpContext.m_Nodes.GetCount() - 1;
  for (ezUInt32 n = 0; n < pNode->mNumChildren; ++n)
  {
    ProcessNodes(pNode->mChildren[n], uiNodeIndex, inout_uiLODCount);
  }
}

void spMeshProcessor::ProcessMeshes(const aiNode* pNode, ezUInt32 uiParentIndex, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex)
{
  const ezUInt32 uiMeshCount = pNode->mNumMeshes;

  // Read this node mesh data
  for (ezUInt32 meshIndex = 0; meshIndex < uiMeshCount; ++meshIndex)
  {
    const aiMesh* pMesh = m_pScene->mMeshes[pNode->mMeshes[meshIndex]];

    // We only handle triangles
    if (pMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
      continue;

    ezHashedString sMeshName;
    sMeshName.Assign(pMesh->mName.C_Str());

    if (m_AssimpContext.m_MeshesNames.Contains(sMeshName))
    {
      ezStringBuilder sTokenizedMeshName;
      const ezUInt32 uiCount = *m_AssimpContext.m_MeshesNames.GetValue(sMeshName) + 1;
      sMeshName.Assign(ezFmt("{}_{}", pMesh->mName.C_Str(), uiCount).GetText(sTokenizedMeshName));
      m_AssimpContext.m_MeshesNames[sMeshName] = uiCount;
    }

    spAssimpMesh& meshNode = m_AssimpContext.m_Meshes.ExpandAndGetRef();

    meshNode.m_sName = sMeshName;
    meshNode.m_uiParentNodeIndex = uiParentIndex;
    meshNode.m_pMesh = pMesh;

    ezUInt32 uiIndexCount = pMesh->mNumFaces * 3; // Import only triangles
    ezUInt32 uiVertexCount = 0;

    m_AssimpContext.m_MeshData.m_Vertices.EnsureCount(out_uiBaseVertex + pMesh->mNumVertices);
    m_AssimpContext.m_MeshData.m_Indices.EnsureCount(out_uiBaseIndex + uiIndexCount);

    meshNode.m_uiBaseIndex = out_uiBaseIndex;
    meshNode.m_uiBaseVertex = out_uiBaseVertex;

    ezMap<ezUInt16, ezUInt16> remappedIndices;

    // Populate the vertex buffer of the mesh data
    for (ezUInt32 vertexIndex = 0; vertexIndex < pMesh->mNumVertices; ++vertexIndex)
    {
      spVertex vertex;

      vertex.m_vPosition = spFromAssimp(pMesh->mVertices[vertexIndex]);
      vertex.m_vNormal = pMesh->HasNormals() ? spFromAssimp(pMesh->mNormals[vertexIndex]) : ezVec3::ZeroVector();
      vertex.m_vTangent = pMesh->HasTangentsAndBitangents() ? spFromAssimp(pMesh->mTangents[vertexIndex]).GetAsVec4(1.0f) : ezVec4::ZeroVector();
      vertex.m_vBiTangent = pMesh->HasTangentsAndBitangents() ? spFromAssimp(pMesh->mBitangents[vertexIndex]).GetAsVec4(1.0f) : ezVec4::ZeroVector();
      vertex.m_vTexCoord0 = pMesh->HasTextureCoords(0) ? spFromAssimp(pMesh->mTextureCoords[0][vertexIndex]).GetAsVec2() : ezVec2::ZeroVector();
      vertex.m_vTexCoord1 = pMesh->HasTextureCoords(1) ? spFromAssimp(pMesh->mTextureCoords[1][vertexIndex]).GetAsVec2() : ezVec2::ZeroVector();
      vertex.m_Color0 = pMesh->HasVertexColors(0) ? spFromAssimp(pMesh->mColors[0][vertexIndex]) : ezColor::White;
      vertex.m_Color1 = pMesh->HasVertexColors(1) ? spFromAssimp(pMesh->mColors[1][vertexIndex]) : ezColor::White;

      if (const ezUInt32 uiFoundVertex = m_AssimpContext.m_MeshData.m_Vertices.IndexOf(vertex, meshNode.m_uiBaseVertex); uiFoundVertex != ezInvalidIndex)
      {
        remappedIndices[vertexIndex] = static_cast<ezUInt16>(uiFoundVertex - meshNode.m_uiBaseVertex);
        continue;
      }

      m_AssimpContext.m_MeshData.m_Vertices[meshNode.m_uiBaseVertex + uiVertexCount] = vertex;

      remappedIndices[vertexIndex] = static_cast<ezUInt16>(uiVertexCount);
      uiVertexCount++;
    }

    // Populate the index buffer of the mesh data
    for (uiIndexCount = 0; uiIndexCount < pMesh->mNumFaces; ++uiIndexCount)
    {
      m_AssimpContext.m_MeshData.m_Indices[meshNode.m_uiBaseIndex + uiIndexCount * 3 + 0] = remappedIndices[static_cast<ezUInt16>(pMesh->mFaces[uiIndexCount].mIndices[0])];
      m_AssimpContext.m_MeshData.m_Indices[meshNode.m_uiBaseIndex + uiIndexCount * 3 + 1] = remappedIndices[static_cast<ezUInt16>(pMesh->mFaces[uiIndexCount].mIndices[1])];
      m_AssimpContext.m_MeshData.m_Indices[meshNode.m_uiBaseIndex + uiIndexCount * 3 + 2] = remappedIndices[static_cast<ezUInt16>(pMesh->mFaces[uiIndexCount].mIndices[2])];
    }

    uiIndexCount *= 3;

    // Clear memory
    remappedIndices.Clear();

    // Perform mesh optimization if required
    OptimizeMeshData(
      (m_AssimpContext.m_MeshData.m_Vertices.GetData() + meshNode.m_uiBaseVertex)->m_vPosition.GetData(),
      uiVertexCount,
      m_AssimpContext.m_MeshData.m_Indices.GetData() + meshNode.m_uiBaseIndex,
      uiIndexCount,
      meshNode.m_MeshOptRemap);

    meshNode.m_uiIndexCount = uiIndexCount;
    meshNode.m_uiVertexCount = uiVertexCount;

    out_uiBaseIndex += uiIndexCount;
    out_uiBaseVertex += uiVertexCount;
  }
}

void spMeshProcessor::ProcessBlendShapes()
{
  const ezUInt32 uiMeshCount = m_AssimpContext.m_Meshes.GetCount();

  // Read this node mesh data
  for (ezUInt32 uiMeshIndex = 0; uiMeshIndex < uiMeshCount; ++uiMeshIndex)
  {
    const spAssimpMesh& assimpMesh = m_AssimpContext.m_Meshes[uiMeshIndex];
    const aiMesh* pMesh = assimpMesh.m_pMesh;

    // Process blend shape data
    for (ezUInt32 uiBlendShapeIndex = 0; uiBlendShapeIndex < pMesh->mNumAnimMeshes; ++uiBlendShapeIndex)
    {
      const aiAnimMesh* pAnimMesh = pMesh->mAnimMeshes[uiBlendShapeIndex];

      spAssimpBlendShape& blendShapeNode = m_AssimpContext.m_BlendShapes.ExpandAndGetRef();
      blendShapeNode.m_uiParentMeshIndex = uiMeshIndex;
      blendShapeNode.m_fWeight = pAnimMesh->mWeight;
      blendShapeNode.m_pAnimMesh = pAnimMesh;

      ezUInt32 uiIndexCount = 0;
      ezUInt32 uiVertexCount = 0;

      blendShapeNode.m_BlendShapeData.EnsureCount(pAnimMesh->mNumVertices);

      // Populate the vertex buffer of the mesh data
      for (ezUInt32 uiVertexIndex = 0; uiVertexIndex < pAnimMesh->mNumVertices; ++uiVertexIndex)
      {
        spVertex vertex;

        vertex.m_vPosition = spFromAssimp(pAnimMesh->mVertices[uiVertexIndex]);
        vertex.m_vNormal = pAnimMesh->HasNormals() ? spFromAssimp(pAnimMesh->mNormals[uiVertexIndex]) : ezVec3::ZeroVector();
        vertex.m_vTangent = pAnimMesh->HasTangentsAndBitangents() ? spFromAssimp(pAnimMesh->mTangents[uiVertexIndex]).GetAsVec4(1.0f) : ezVec4::ZeroVector();
        vertex.m_vBiTangent = pAnimMesh->HasTangentsAndBitangents() ? spFromAssimp(pAnimMesh->mBitangents[uiVertexIndex]).GetAsVec4(1.0f) : ezVec4::ZeroVector();
        vertex.m_vTexCoord0 = pAnimMesh->HasTextureCoords(0) ? spFromAssimp(pAnimMesh->mTextureCoords[0][uiVertexIndex]).GetAsVec2() : ezVec2::ZeroVector();
        vertex.m_vTexCoord1 = pAnimMesh->HasTextureCoords(1) ? spFromAssimp(pAnimMesh->mTextureCoords[1][uiVertexIndex]).GetAsVec2() : ezVec2::ZeroVector();
        vertex.m_Color0 = pAnimMesh->HasVertexColors(0) ? spFromAssimp(pAnimMesh->mColors[0][uiVertexIndex]) : ezColor::White;
        vertex.m_Color1 = pAnimMesh->HasVertexColors(1) ? spFromAssimp(pAnimMesh->mColors[1][uiVertexIndex]) : ezColor::White;

        if (const ezUInt32 uiFoundVertex = blendShapeNode.m_BlendShapeData.IndexOf(vertex); uiFoundVertex != ezInvalidIndex)
          continue;

        blendShapeNode.m_BlendShapeData[uiVertexCount] = vertex;
        uiVertexCount++;
      }

      blendShapeNode.m_BlendShapeData.SetCount(uiVertexCount);
      blendShapeNode.m_BlendShapeData.Compact();

      // Perform mesh optimization if required
      if (m_Configuration.m_AssimpImporterConfig.m_bOptimizeMesh)
      {
        meshopt_remapVertexBuffer(
          blendShapeNode.m_BlendShapeData.GetData()->m_vPosition.GetData(),
          blendShapeNode.m_BlendShapeData.GetData()->m_vPosition.GetData(),
          uiVertexCount,
          sizeof(spVertex),
          assimpMesh.m_MeshOptRemap.GetData());
      }

      // Recompute tangent space if required
      if (m_Configuration.m_AssimpImporterConfig.m_bRecomputeTangents)
      {
        const auto& indices = m_AssimpContext.m_MeshData.m_Indices.GetArrayPtr().GetSubArray(assimpMesh.m_uiBaseIndex, assimpMesh.m_uiIndexCount);
        RecomputeTangents(blendShapeNode.m_BlendShapeData, indices);
      }
    }
  }
}

void spMeshProcessor::OptimizeMeshData(float* pVertexBuffer, ezUInt32 uiVertexCount, ezUInt16* pIndexBuffer, ezUInt32 uiIndexCount, ezDynamicArray<ezUInt32>& ref_remap) const
{
  if (!m_Configuration.m_AssimpImporterConfig.m_bOptimizeMesh)
    return;

  ref_remap.SetCountUninitialized(uiVertexCount);

  meshopt_optimizeVertexCache(
    pIndexBuffer,
    pIndexBuffer,
    uiIndexCount,
    uiVertexCount);

  meshopt_optimizeOverdraw(
    pIndexBuffer,
    pIndexBuffer,
    uiIndexCount,
    pVertexBuffer,
    uiVertexCount,
    sizeof(spVertex),
    kMeshOptimizationThreshold);

  meshopt_optimizeVertexFetchRemap(
    ref_remap.GetData(),
    pIndexBuffer,
    uiIndexCount,
    uiVertexCount);

  meshopt_remapIndexBuffer(
    pIndexBuffer,
    pIndexBuffer,
    uiIndexCount,
    ref_remap.GetData());

  meshopt_remapVertexBuffer(
    pVertexBuffer,
    pVertexBuffer,
    uiVertexCount,
    sizeof(spVertex),
    ref_remap.GetData());
}
