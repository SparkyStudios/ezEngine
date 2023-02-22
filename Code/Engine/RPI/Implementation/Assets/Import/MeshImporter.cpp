#include <RPI/RPIPCH.h>

#include <RPI/Assets/Import/MeshImporter.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meshoptimizer.h>

#include <mikktspace/mikktspace.h>

class aiLogStreamError : public Assimp::LogStream
{
public:
  void write(const char* message) { ezLog::Warning("AssImp: {0}", message); }
};

class aiLogStreamWarning : public Assimp::LogStream
{
public:
  void write(const char* message) { ezLog::Warning("AssImp: {0}", message); }
};

class aiLogStreamInfo : public Assimp::LogStream
{
public:
  void write(const char* message) { ezLog::Dev("AssImp: {0}", message); }
};

static ezColor spFromAssimp(const aiColor4D& value, bool bInvert = false)
{
  if (bInvert)
    return {1.0f - value.r, 1.0f - value.g, 1.0f - value.b, 1.0f - value.a};

  return {value.r, value.g, value.b, value.a};
}

static ezColor spFromAssimp(const aiColor3D& value, bool bInvert = false)
{
  if (bInvert)
    return {1.0f - value.r, 1.0f - value.g, 1.0f - value.b};

  return {value.r, value.g, value.b};
}

static ezMat4 spFromAssimp(const aiMatrix4x4& value)
{
  ezMat4 mTransformation;
  mTransformation.SetFromArray(&value.a1, ezMatrixLayout::RowMajor);
  return mTransformation;
}

static ezVec3 spFromAssimp(const aiVector3D& value)
{
  return {value.x, value.y, value.z};
}

static ezQuat spFromAssimp(const aiQuaternion& value)
{
  return {value.x, value.y, value.z, value.w};
}

static float spFromAssimp(float value)
{
  return value;
}

static int spFromAssimp(int value)
{
  return value;
}

static int MikkTGetNumFaces(const SMikkTSpaceContext* pContext)
{
  const auto* pMeshData = static_cast<spMesh::Data*>(pContext->m_pUserData);

  const auto fSize = static_cast<float>(pMeshData->m_Indices.GetCount()) / 3.0f;
  const auto iSize = static_cast<int>(pMeshData->m_Indices.GetCount()) / 3;

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
  const auto* pMeshData = static_cast<spMesh::Data*>(pContext->m_pUserData);

  const int iFaceSize = MikkTGetNumVerticesOfFace(pContext, iFace);
  const int iIndicesIndex = iFace * iFaceSize + iVert;

  return pMeshData->m_Indices[iIndicesIndex];
}

static void MikkTGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMesh::Data*>(pContext->m_pUserData);

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const spMesh::Vertex vertex = pMeshData->m_Vertices[iIndex];

  fvNormOut[0] = vertex.m_vNormal.x;
  fvNormOut[1] = vertex.m_vNormal.y;
  fvNormOut[2] = vertex.m_vNormal.z;
}

static void MikkTGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMesh::Data*>(pContext->m_pUserData);

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const spMesh::Vertex vertex = pMeshData->m_Vertices[iIndex];

  fvPosOut[0] = vertex.m_vPosition.x;
  fvPosOut[1] = vertex.m_vPosition.y;
  fvPosOut[2] = vertex.m_vPosition.z;
}

static void MikkTGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMesh::Data*>(pContext->m_pUserData);

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const spMesh::Vertex vertex = pMeshData->m_Vertices[iIndex];

  fvTexcOut[0] = vertex.m_vTexCoord0.x;
  fvTexcOut[1] = vertex.m_vTexCoord0.y;
}

static void MikkTSetTSpace(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT, const tbool bIsOrientationPreserving, const int iFace, const int iVert)
{
  auto* pMeshData = static_cast<spMesh::Data*>(pContext->m_pUserData);

  const auto index = MikkTGetVertexIndex(pContext, iFace, iVert);
  auto* vertex = &pMeshData->m_Vertices[index];

  vertex->m_vTangent.x = fvTangent[0];
  vertex->m_vTangent.y = fvTangent[1];
  vertex->m_vTangent.z = fvTangent[2];
  vertex->m_vTangent.w = fMagS;

  vertex->m_vBiTangent.x = fvBiTangent[0];
  vertex->m_vBiTangent.y = fvBiTangent[1];
  vertex->m_vBiTangent.z = fvBiTangent[2];
  vertex->m_vBiTangent.w = fMagT;
}

ezResult spMeshImporter::Import(ezStringView sSourceFile, spMesh* out_pAsset)
{
  EZ_LOG_BLOCK("ezMeshImporter::Import()");

  m_sFilePath = sSourceFile;

  ezUInt32 uiPreset = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FindDegenerates | aiProcess_FindInstances | aiProcess_FindInvalidData | aiProcess_RemoveRedundantMaterials
                      | aiProcess_LimitBoneWeights | aiProcess_GlobalScale | aiProcess_JoinIdenticalVertices | aiProcess_SplitLargeMeshes | aiProcess_RemoveComponent | aiProcess_GenUVCoords | aiProcess_TransformUVCoords;

  if (m_Configuration.m_bFlipWindingNormals)
    uiPreset |= aiProcess_FixInfacingNormals;
  if (m_Configuration.m_bFlipUVs)
    uiPreset |= aiProcess_FlipUVs;
  if (!m_Configuration.m_bImportSkeleton)
    uiPreset |= aiProcess_Debone;
  if (m_Configuration.m_bRecomputeNormals)
    uiPreset |= aiProcess_ForceGenNormals;

  ezStringBuilder sb;
  const aiScene* pScene = m_Importer.ReadFile(sSourceFile.GetData(sb), uiPreset);
  if (pScene == nullptr)
  {
    ezLog::Error("Assimp failed to import '{}'", sSourceFile);
    return EZ_FAILURE;
  }

  if (m_Configuration.m_bImportMaterials)
  {
    // ImportMaterials(pScene);
  }

  if (m_Configuration.m_bImportMeshes)
  {
    ImportMeshes(pScene, out_pAsset);
  }

  if (m_Configuration.m_bImportSkeleton)
  {
    // ImportSkeleton(pScene);
  }

  if (m_Configuration.m_bImportAnimations)
  {
    // ImportAnimations(pScene);
  }

  return EZ_SUCCESS;
}

spMeshImporter::spMeshImporter(const spMeshImporterConfiguration& configuration)
  : spImporter(configuration)
{
  Assimp::DefaultLogger::create("", Assimp::Logger::NORMAL);

  Assimp::DefaultLogger::get()->attachStream(new aiLogStreamError(), Assimp::Logger::Err);
  Assimp::DefaultLogger::get()->attachStream(new aiLogStreamWarning(), Assimp::Logger::Warn);
  Assimp::DefaultLogger::get()->attachStream(new aiLogStreamInfo(), Assimp::Logger::Info);

  m_Importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, configuration.m_fScale);
  m_Importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
  m_Importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
  m_Importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, UINT16_MAX);

  ezInt32 uiExcludeFlags = aiComponent_CAMERAS | aiComponent_LIGHTS;

  if (!configuration.m_bImportAnimations)
    uiExcludeFlags |= aiComponent_ANIMATIONS;

  if (!configuration.m_bImportSkeleton)
    uiExcludeFlags |= aiComponent_BONEWEIGHTS;

  if (!configuration.m_bImportMaterials)
    uiExcludeFlags |= aiComponent_MATERIALS | aiComponent_TEXTURES;

  if (!configuration.m_bImportMeshes)
    uiExcludeFlags |= aiComponent_MESHES;

  m_Importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, uiExcludeFlags);
}

spMeshImporter::~spMeshImporter()
{
}

void spMeshImporter::ImportMeshes(const aiScene* pScene, spMesh* out_pMesh)
{
  const ezUInt32 uiMeshCount = pScene->mNumMeshes;

  ezDynamicArray<spMesh::Entry> entries;
  entries.SetCount(uiMeshCount);

  ezUInt32 uiBaseVertex = 0;
  ezUInt32 uiBaseIndex = 0;

  // Count the number of vertices and indices
  for (ezUInt32 meshIndex = 0; meshIndex < uiMeshCount; ++meshIndex)
  {
    const aiMesh* pMesh = pScene->mMeshes[meshIndex];
    const ezUInt32 uiIndicesCount = pMesh->mNumFaces * 3; // Import only triangles

    entries[meshIndex].m_sName = pMesh->mName.C_Str();
    entries[meshIndex].m_uiBaseIndex = uiBaseIndex;
    entries[meshIndex].m_uiIndicesCount = uiIndicesCount;
    entries[meshIndex].m_uiBaseVertex = uiBaseVertex;
    entries[meshIndex].m_uiVerticesCount = pMesh->mNumVertices;

    if (m_Configuration.m_bImportMaterials)
    {
      // TODO: Material
    }

    uiBaseIndex += uiIndicesCount;
    uiBaseVertex += pMesh->mNumVertices;
  }

  spMesh::Data meshData;

  meshData.m_Vertices.SetCount(uiBaseVertex);
  meshData.m_Indices.SetCount(uiBaseIndex);

  // Build mesh data
  for (ezUInt32 meshIndex = 0; meshIndex < uiMeshCount; ++meshIndex)
  {
    ImportMesh(meshData, pScene->mMeshes[meshIndex]);
  }

  if (m_Configuration.m_bRecomputeTangents)
    RecomputeTangents(meshData);

  out_pMesh->SetData(meshData);
  out_pMesh->SetRootNode(ComputeMeshHierarchy(entries, pScene->mRootNode));
}

spMesh::Node spMeshImporter::ComputeMeshHierarchy(const ezDynamicArray<spMesh::Entry>& allEntries, const aiNode* pRootNode)
{
  spMesh::Node node;

  aiVector3D s, r, p;
  pRootNode->mTransformation.Decompose(s, r, p);

  node.m_sName = pRootNode->mName.C_Str();
  node.m_Transform = {spFromAssimp(p), spFromAssimp(s), spFromAssimp(r)};

  if (pRootNode->mNumMeshes > 0)
    for (ezUInt32 meshIndex = 0; meshIndex < pRootNode->mNumMeshes; ++meshIndex)
      node.m_Entries.ExpandAndGetRef() = allEntries[pRootNode->mMeshes[meshIndex]];

  if (pRootNode->mNumChildren > 0)
    for (ezUInt32 childIndex = 0; childIndex < pRootNode->mNumChildren; ++childIndex)
      node.m_Children.ExpandAndGetRef() = ComputeMeshHierarchy(allEntries, pRootNode->mChildren[childIndex]);

  return node;
}

void spMeshImporter::ImportMesh(spMesh::Data& data, const aiMesh* pMesh)
{
  // We only handle triangles
  if (pMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
    return;

  // Import bones if enabled
  if (m_Configuration.m_bImportSkeleton && pMesh->HasBones())
  {
    for (ezUInt32 boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex)
    {
      // TODO: Bone Import
    }
  }

  // Populate the vertex buffer of the mesh data
  for (ezUInt32 vertexIndex = 0; vertexIndex < pMesh->mNumVertices; ++vertexIndex)
  {
    data.m_Vertices[vertexIndex].m_vPosition = spFromAssimp(pMesh->mVertices[vertexIndex]);
    data.m_Vertices[vertexIndex].m_vNormal = pMesh->HasNormals() ? spFromAssimp(pMesh->mNormals[vertexIndex]) : ezVec3::ZeroVector();
    data.m_Vertices[vertexIndex].m_vTangent = pMesh->HasTangentsAndBitangents() ? spFromAssimp(pMesh->mTangents[vertexIndex]).GetAsVec4(1.0f) : ezVec4::ZeroVector();
    data.m_Vertices[vertexIndex].m_vBiTangent = pMesh->HasTangentsAndBitangents() ? spFromAssimp(pMesh->mBitangents[vertexIndex]).GetAsVec4(1.0f) : ezVec4::ZeroVector();
    data.m_Vertices[vertexIndex].m_vTexCoord0 = pMesh->HasTextureCoords(0) ? spFromAssimp(pMesh->mTextureCoords[0][vertexIndex]).GetAsVec2() : ezVec2::ZeroVector();
    data.m_Vertices[vertexIndex].m_vTexCoord1 = pMesh->HasTextureCoords(1) ? spFromAssimp(pMesh->mTextureCoords[1][vertexIndex]).GetAsVec2() : ezVec2::ZeroVector();
    data.m_Vertices[vertexIndex].m_Color0 = pMesh->HasVertexColors(0) ? spFromAssimp(pMesh->mColors[0][vertexIndex]) : ezColor::White;
    data.m_Vertices[vertexIndex].m_Color1 = pMesh->HasVertexColors(1) ? spFromAssimp(pMesh->mColors[1][vertexIndex]) : ezColor::White;
  }

  // Populate the index buffer of the mesh data
  for (ezUInt32 indexIndex = 0; indexIndex < pMesh->mNumFaces; ++indexIndex)
  {
    data.m_Indices[indexIndex * 3 + 0] = static_cast<ezUInt16>(pMesh->mFaces[indexIndex].mIndices[0]);
    data.m_Indices[indexIndex * 3 + 1] = static_cast<ezUInt16>(pMesh->mFaces[indexIndex].mIndices[1]);
    data.m_Indices[indexIndex * 3 + 2] = static_cast<ezUInt16>(pMesh->mFaces[indexIndex].mIndices[2]);
  }

  // Perform mesh optimization if required
  if (m_Configuration.m_bOptimizeMesh)
  {
    constexpr float kThreshold = 1.01f; // allow up to 1% worse ACMR to get more reordering opportunities for overdraw

    meshopt_optimizeVertexCache(
      data.m_Indices.GetData(),
      data.m_Indices.GetData(),
      data.m_Indices.GetCount(),
      data.m_Vertices.GetCount());

    meshopt_optimizeOverdraw(
      data.m_Indices.GetData(),
      data.m_Indices.GetData(),
      data.m_Indices.GetCount(),
      &data.m_Vertices.GetData()->m_vPosition.x,
      data.m_Vertices.GetCount(),
      sizeof(spMesh::Vertex),
      kThreshold);

    ezDynamicArray<ezUInt32> remap;
    remap.SetCountUninitialized(data.m_Vertices.GetCount());

    meshopt_optimizeVertexFetchRemap(
      remap.GetData(),
      data.m_Indices.GetData(),
      data.m_Indices.GetCount(),
      data.m_Vertices.GetCount());

    meshopt_remapIndexBuffer(
      data.m_Indices.GetData(),
      data.m_Indices.GetData(),
      data.m_Indices.GetCount(),
      remap.GetData());

    meshopt_remapVertexBuffer(
      data.m_Vertices.GetData(),
      data.m_Vertices.GetData(),
      data.m_Vertices.GetCount(),
      sizeof(spMesh::Vertex),
      remap.GetData());

    remap.Clear();
  }
}

void spMeshImporter::RecomputeTangents(spMesh::Data& data)
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

  mktContext.m_pInterface = &mkt;
  mktContext.m_pUserData = &data;

  genTangSpaceDefault(&mktContext);
}
