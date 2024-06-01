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

#include <AssetProcessor/Processors/MeshProcessor.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meshoptimizer.h>

#include <mikktspace.h>

using namespace RAI;


static constexpr float kMeshOptimizationThreshold = 1.01f; // allow up to 1% worse ACMR to get more reordering opportunities for overdraw

#pragma region MikkT Tangent Space Generation

struct spMikkTSpaceContext
{
  spMesh::Data* m_pMeshData{nullptr};

  ezUInt32 m_uiPositionStreamIndex{ezInvalidIndex};
  ezUInt32 m_uiNormalStreamIndex{ezInvalidIndex};
  ezUInt32 m_uiTexCoord0StreamIndex{ezInvalidIndex};
  ezUInt32 m_uiTangentStreamIndex{ezInvalidIndex};
  ezUInt32 m_uiBiTangentStreamIndex{ezInvalidIndex};
};

static int MikkTGetNumFaces(const SMikkTSpaceContext* pContext)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_pMeshData;
  const auto& uiIndexCount = pMeshData->GetIndexCount();

  const auto fSize = static_cast<float>(uiIndexCount) / 3.0f;
  const auto iSize = static_cast<int>(uiIndexCount) / 3;

  EZ_ASSERT_DEV(fSize - static_cast<float>(iSize) == 0.0f, "There are faces in the mesh that are not triangles.");

  return iSize;
}

static int MikkTGetNumVerticesOfFace(const SMikkTSpaceContext* context, ezInt32 iFace)
{
  // We only work with triangles
  return 3;
}

static int MikkTGetVertexIndex(const SMikkTSpaceContext* pContext, ezInt32 iFace, ezInt32 iVert)
{
  auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_pMeshData;

  const int iFaceSize = MikkTGetNumVerticesOfFace(pContext, iFace);
  const int iIndicesIndex = iFace * iFaceSize + iVert;

  return reinterpret_cast<ezUInt16*>(pMeshData->m_Indices.GetData())[iIndicesIndex];
}

static void MikkTGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_pMeshData;
  const auto& uiStreamIndex = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_uiNormalStreamIndex;

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const ezConstByteArrayPtr& pNormal = pMeshData->GetVertexStreamData(uiStreamIndex, iIndex);

  auto* pDest = reinterpret_cast<ezVec3*>(fvNormOut);
  spMeshDataUtils::DecodeNormal(pNormal, pMeshData->GetStreamFormat(uiStreamIndex), *pDest).IgnoreResult();
}

static void MikkTGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_pMeshData;
  const auto& uiStreamIndex = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_uiPositionStreamIndex;

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const ezConstByteArrayPtr& pPosition = pMeshData->GetVertexStreamData(uiStreamIndex, iIndex);

  auto* pDest = reinterpret_cast<ezVec3*>(fvPosOut);
  spMeshDataUtils::DecodeVec3(pPosition, pMeshData->GetStreamFormat(uiStreamIndex), *pDest).IgnoreResult();
}

static void MikkTGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
  const auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_pMeshData;
  const auto& uiStreamIndex = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_uiTexCoord0StreamIndex;

  const int iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const ezConstByteArrayPtr& pTexCoord = pMeshData->GetVertexStreamData(uiStreamIndex, iIndex);

  auto* pDest = reinterpret_cast<ezVec2*>(fvTexcOut);
  spMeshDataUtils::DecodeVec2(pTexCoord, pMeshData->GetStreamFormat(uiStreamIndex), *pDest).IgnoreResult();
}

static void MikkTSetTSpace(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT, const tbool bIsOrientationPreserving, const int iFace, const int iVert)
{
  /* */ auto* pMeshData = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_pMeshData;
  const auto& uiTangentStreamIndex = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_uiTangentStreamIndex;
  const auto& uiBiTangentStreamIndex = static_cast<spMikkTSpaceContext*>(pContext->m_pUserData)->m_uiBiTangentStreamIndex;

  const auto iIndex = MikkTGetVertexIndex(pContext, iFace, iVert);
  const ezByteArrayPtr& pTangent = pMeshData->GetVertexStreamData(uiTangentStreamIndex, iIndex);
  const ezByteArrayPtr& pBiTangent = pMeshData->GetVertexStreamData(uiBiTangentStreamIndex, iIndex);

  const ezVec3& vTangent = *reinterpret_cast<const ezVec3*>(fvTangent);
  const ezVec3& vBiTangent = *reinterpret_cast<const ezVec3*>(fvBiTangent);

  spMeshDataUtils::EncodeTangent(vTangent, fMagS, pTangent, pMeshData->GetStreamFormat(uiTangentStreamIndex)).IgnoreResult();
  spMeshDataUtils::EncodeTangent(vBiTangent, fMagT, pBiTangent, pMeshData->GetStreamFormat(uiBiTangentStreamIndex)).IgnoreResult();
}

static void RecomputeTangents(
  spMesh::Data* pMeshData,
  ezUInt32 uiPositionStreamIndex,
  ezUInt32 uiNormalStreamIndex,
  ezUInt32 uiTexCoord0StreamIndex,
  ezUInt32 uiTangentStreamIndex,
  ezUInt32 uiBiTangentStreamIndex)
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
  context.m_pMeshData = pMeshData;
  context.m_uiPositionStreamIndex = uiPositionStreamIndex;
  context.m_uiNormalStreamIndex = uiNormalStreamIndex;
  context.m_uiTexCoord0StreamIndex = uiTexCoord0StreamIndex;
  context.m_uiTangentStreamIndex = uiTangentStreamIndex;
  context.m_uiBiTangentStreamIndex = uiBiTangentStreamIndex;

  mktContext.m_pInterface = &mkt;
  mktContext.m_pUserData = &context;

  genTangSpaceDefault(&mktContext);
}

#pragma endregion

#pragma region Assimp Logging

class aiLogStreamError final : public Assimp::LogStream
{
public:
  void write(const char* message) override { ezLog::Error("AssImp: {0}", message); }
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

#pragma region Utilities

static RHI::spInputElementFormat::Enum GetNormalInputFormat(const ezEnum<spAssimpVertexStreamComponentPrecision>& ePrecision)
{
  switch (ePrecision)
  {
    case spAssimpVertexStreamComponentPrecision::_10Bit:
      return RHI::spInputElementFormat::R10G10B10A2UNorm;

    case spAssimpVertexStreamComponentPrecision::_16Bit:
      return RHI::spInputElementFormat::UShort4Norm;

    default:
    case spAssimpVertexStreamComponentPrecision::_32Bit:
      return RHI::spInputElementFormat::Float3;
  }
}

static RHI::spInputElementFormat::Enum GetTangentInputFormat(const ezEnum<spAssimpVertexStreamComponentPrecision>& ePrecision)
{
  switch (ePrecision)
  {
    case spAssimpVertexStreamComponentPrecision::_10Bit:
      return RHI::spInputElementFormat::R10G10B10A2UNorm;

    case spAssimpVertexStreamComponentPrecision::_16Bit:
      return RHI::spInputElementFormat::UShort4Norm;

    default:
    case spAssimpVertexStreamComponentPrecision::_32Bit:
      return RHI::spInputElementFormat::Float4;
  }
}

static RHI::spInputElementFormat::Enum GetTexCoordInputFormat(const ezEnum<spAssimpVertexStreamComponentPrecision>& ePrecision)
{
  switch (ePrecision)
  {
    case spAssimpVertexStreamComponentPrecision::_16Bit:
      return RHI::spInputElementFormat::Half2;

    default:
    case spAssimpVertexStreamComponentPrecision::_32Bit:
      return RHI::spInputElementFormat::Float2;
  }
}

static RHI::spInputElementFormat::Enum GetBoneWeightInputFormat(const ezEnum<spAssimpVertexStreamComponentPrecision>& ePrecision)
{
  switch (ePrecision)
  {
    case spAssimpVertexStreamComponentPrecision::_8Bit:
      return RHI::spInputElementFormat::Byte4Norm;

    case spAssimpVertexStreamComponentPrecision::_10Bit:
      return RHI::spInputElementFormat::R10G10B10A2UNorm;

    case spAssimpVertexStreamComponentPrecision::_16Bit:
      return RHI::spInputElementFormat::UShort4Norm;

    default:
    case spAssimpVertexStreamComponentPrecision::_32Bit:
      return RHI::spInputElementFormat::Float4;
  }
}

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

  ezUInt32 uiPreset = aiProcess_SplitLargeMeshes | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FindDegenerates | aiProcess_FindInstances | aiProcess_FindInvalidData | aiProcess_RemoveComponent | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_GlobalScale;

  if (m_Configuration.m_AssimpImporterConfig.m_bFlipWindingNormals)
    uiPreset |= aiProcess_FixInfacingNormals;
  if (m_Configuration.m_AssimpImporterConfig.m_bFlipUVs)
    uiPreset |= aiProcess_FlipUVs;
  if (m_Configuration.m_AssimpImporterConfig.m_bRecomputeNormals)
    uiPreset |= aiProcess_ForceGenNormals | aiProcess_GenSmoothNormals;

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

    ProcessNodes(m_pScene->mRootNode, ezInvalidIndex, uiLODCount);

    m_AssimpContext.m_uiLODCount = uiLODCount + 1;
  }

  // Set up the mesh data builder
  SetupMeshDataBuilder();

  // Process mesh data
  if (m_Configuration.m_bImportMeshes)
  {
    ezUInt32 uiBaseVertex = 0;
    ezUInt32 uiBaseIndex = 0;

    // Collect mesh nodes
    for (ezUInt32 n = 0, l = m_AssimpContext.m_Nodes.GetCount(); n < l; ++n)
      CollectMeshes(m_AssimpContext.m_Nodes[n].m_pNode, n, uiBaseVertex, uiBaseIndex);

    // Allocate mesh data
    m_AssimpContext.m_MeshDataBuilder.AllocateMeshData(
      m_AssimpContext.m_MeshData,
      m_AssimpContext.m_uiVertexCount,
      RHI::spPrimitiveTopology::Triangles,
      m_AssimpContext.m_uiPrimitiveCount,
      true);

    // Process mesh data
    ProcessMeshes();

    // Recompute tangent space if required
    if (m_Configuration.m_AssimpImporterConfig.m_bRecomputeTangents)
      RecomputeTangents(
        &m_AssimpContext.m_MeshData,
        m_AssimpContext.m_Streams.m_uiPositionStreamIndex,
        m_AssimpContext.m_Streams.m_uiNormalStreamIndex,
        m_AssimpContext.m_Streams.m_uiTexCoord0StreamIndex,
        m_AssimpContext.m_Streams.m_uiTangentStreamIndex,
        m_AssimpContext.m_Streams.m_uiBiTangentStreamIndex);

    // Process blend shape data
    if (m_Configuration.m_bImportMotions)
      ProcessBlendShapes();
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

  const ezMat4 mParentTransform = uiParentIndex == ezInvalidIndex ? ezMat4::MakeIdentity() : m_AssimpContext.m_Nodes[uiParentIndex].m_mGlobalTransform;
  const ezMat4 mLocalTransform = spFromAssimp(pNode->mTransformation);
  node.m_mGlobalTransform = mParentTransform * mLocalTransform;

  const ezUInt32 uiNodeIndex = m_AssimpContext.m_Nodes.GetCount() - 1;
  for (ezUInt32 n = 0; n < pNode->mNumChildren; ++n)
    ProcessNodes(pNode->mChildren[n], uiNodeIndex, inout_uiLODCount);
}

void spMeshProcessor::SetupMeshDataBuilder()
{
  m_AssimpContext.m_Streams.m_uiPositionStreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Position, 0, RHI::spInputElementFormat::Float3);
  m_AssimpContext.m_Streams.m_uiNormalStreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Normal, 0, GetNormalInputFormat(m_Configuration.m_AssimpImporterConfig.m_eNormalPrecision));
  m_AssimpContext.m_Streams.m_uiTangentStreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Tangent, 0, GetTangentInputFormat(m_Configuration.m_AssimpImporterConfig.m_eNormalPrecision));
  m_AssimpContext.m_Streams.m_uiBiTangentStreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::BiTangent, 0, GetTangentInputFormat(m_Configuration.m_AssimpImporterConfig.m_eNormalPrecision));

  m_AssimpContext.m_Streams.m_uiTexCoord0StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 0, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));

  bool bHasTexCoord1 = false, bHasTexCoord2 = false, bHasTexCoord3 = false, bHasTexCoord4 = false, bHasTexCoord5 = false, bHasTexCoord6 = false, bHasTexCoord7 = false;
  bool bHasColor0 = false, bHasColor1 = false, bHasColor2 = false, bHasColor3 = false, bHasColor4 = false, bHasColor5 = false, bHasColor6 = false, bHasColor7 = false;

  for (const auto& mesh : m_AssimpContext.m_Meshes)
  {
    if (!bHasColor1 && mesh.m_pMesh->HasTextureCoords(1))
      bHasTexCoord1 = true;
    if (!bHasColor2 && mesh.m_pMesh->HasTextureCoords(2))
      bHasTexCoord2 = true;
    if (!bHasColor3 && mesh.m_pMesh->HasTextureCoords(3))
      bHasTexCoord3 = true;
    if (!bHasColor4 && mesh.m_pMesh->HasTextureCoords(4))
      bHasTexCoord4 = true;
    if (!bHasColor5 && mesh.m_pMesh->HasTextureCoords(5))
      bHasTexCoord5 = true;
    if (!bHasColor6 && mesh.m_pMesh->HasTextureCoords(6))
      bHasTexCoord6 = true;
    if (!bHasColor7 && mesh.m_pMesh->HasTextureCoords(7))
      bHasTexCoord7 = true;

    if (!bHasColor0 && mesh.m_pMesh->HasVertexColors(0))
      bHasColor0 = true;
    if (!bHasColor1 && mesh.m_pMesh->HasVertexColors(1))
      bHasColor1 = true;
    if (!bHasColor2 && mesh.m_pMesh->HasVertexColors(2))
      bHasColor2 = true;
    if (!bHasColor3 && mesh.m_pMesh->HasVertexColors(3))
      bHasColor3 = true;
    if (!bHasColor4 && mesh.m_pMesh->HasVertexColors(4))
      bHasColor4 = true;
    if (!bHasColor5 && mesh.m_pMesh->HasVertexColors(5))
      bHasColor5 = true;
    if (!bHasColor6 && mesh.m_pMesh->HasVertexColors(6))
      bHasColor6 = true;
    if (!bHasColor7 && mesh.m_pMesh->HasVertexColors(7))
      bHasColor7 = true;
  }

  if (bHasTexCoord1)
    m_AssimpContext.m_Streams.m_uiTexCoord1StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 1, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));
  if (bHasTexCoord2)
    m_AssimpContext.m_Streams.m_uiTexCoord2StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 2, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));
  if (bHasTexCoord3)
    m_AssimpContext.m_Streams.m_uiTexCoord3StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 3, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));
  if (bHasTexCoord4)
    m_AssimpContext.m_Streams.m_uiTexCoord4StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 4, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));
  if (bHasTexCoord5)
    m_AssimpContext.m_Streams.m_uiTexCoord5StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 5, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));
  if (bHasTexCoord6)
    m_AssimpContext.m_Streams.m_uiTexCoord6StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 6, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));
  if (bHasTexCoord7)
    m_AssimpContext.m_Streams.m_uiTexCoord7StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::TexCoord, 7, GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision));

  if (bHasColor0)
    m_AssimpContext.m_Streams.m_uiColor0StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 0, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor1)
    m_AssimpContext.m_Streams.m_uiColor1StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 1, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor2)
    m_AssimpContext.m_Streams.m_uiColor2StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 2, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor3)
    m_AssimpContext.m_Streams.m_uiColor3StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 3, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor4)
    m_AssimpContext.m_Streams.m_uiColor4StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 4, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor5)
    m_AssimpContext.m_Streams.m_uiColor5StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 5, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor6)
    m_AssimpContext.m_Streams.m_uiColor6StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 6, RHI::spInputElementFormat::Byte4Norm);
  if (bHasColor7)
    m_AssimpContext.m_Streams.m_uiColor7StreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::Color, 7, RHI::spInputElementFormat::Byte4Norm);

  if (m_Configuration.m_bImportSkeleton)
  {
    // Needed for skinning
    m_AssimpContext.m_Streams.m_uiBoneWeightsStreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::BoneWeights, 0, GetBoneWeightInputFormat(m_Configuration.m_AssimpImporterConfig.m_eBoneWeightPrecision));
    m_AssimpContext.m_Streams.m_uiBoneIndicesStreamIndex = m_AssimpContext.m_MeshDataBuilder.AddVertexStream(RHI::spInputElementLocationSemantic::BoneIndices, 0, RHI::spInputElementFormat::UShort4);
  }
}

void spMeshProcessor::CollectMeshes(const aiNode* pNode, ezUInt32 uiParentIndex, ezUInt32& out_uiBaseVertex, ezUInt32& out_uiBaseIndex)
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
      m_AssimpContext.m_MeshesNames[sMeshName] = uiCount;

      sMeshName.Assign(ezFmt("{}_{}", pMesh->mName.C_Str(), uiCount).GetText(sTokenizedMeshName));
    }

    spAssimpMesh& meshNode = m_AssimpContext.m_Meshes.ExpandAndGetRef();

    meshNode.m_sName = sMeshName;
    meshNode.m_uiParentNodeIndex = uiParentIndex;
    meshNode.m_pMesh = pMesh;
    meshNode.m_uiBaseIndex = out_uiBaseIndex;
    meshNode.m_uiBaseVertex = out_uiBaseVertex;
    meshNode.m_uiIndexCount = RHI::spPrimitiveTopology::GetIndicesCount(RHI::spPrimitiveTopology::Triangles, pMesh->mNumFaces);
    meshNode.m_uiVertexCount = pMesh->mNumVertices;

    out_uiBaseIndex += meshNode.m_uiIndexCount;
    out_uiBaseVertex += meshNode.m_uiVertexCount;

    m_AssimpContext.m_uiVertexCount = out_uiBaseVertex;
    m_AssimpContext.m_uiPrimitiveCount += pMesh->mNumFaces;
  }
}

void spMeshProcessor::ProcessMeshes()
{
  for (auto& mesh : m_AssimpContext.m_Meshes)
  {
    const aiMesh* pMesh = mesh.m_pMesh;
    const spAssimpNode& meshNode = m_AssimpContext.m_Nodes[mesh.m_uiParentNodeIndex];

    ezMat3 normalsTransform = meshNode.m_mGlobalTransform.GetRotationalPart();
    if (normalsTransform.Invert(0.0f).Failed())
    {
      ezLog::Warning("Couldn't invert a mesh's transform matrix.");
      normalsTransform.SetIdentity();
    }

    normalsTransform.Transpose();

    // Populate the vertex buffer of the mesh data
    for (ezUInt32 vertexIndex = 0; vertexIndex < pMesh->mNumVertices; ++vertexIndex)
    {
      const ezUInt32 uiFinalVertIndex = mesh.m_uiBaseVertex + vertexIndex;

      const ezVec3 vPosition = spFromAssimp(pMesh->mVertices[vertexIndex]);

      m_AssimpContext.m_MeshData.SetVertexStreamData(
        m_AssimpContext.m_Streams.m_uiPositionStreamIndex,
        uiFinalVertIndex,
        vPosition);

      if (m_AssimpContext.m_Streams.m_uiNormalStreamIndex != ezInvalidIndex && pMesh->HasNormals())
      {
        ezVec3 vNormal = normalsTransform * spFromAssimp(pMesh->mNormals[vertexIndex]);
        vNormal.NormalizeIfNotZero(ezVec3::MakeZero()).IgnoreResult();

        spMeshDataUtils::EncodeNormal(
          vNormal,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiNormalStreamIndex, uiFinalVertIndex),
          GetNormalInputFormat(m_Configuration.m_AssimpImporterConfig.m_eNormalPrecision))
          .IgnoreResult();
      }

      if (pMesh->HasTangentsAndBitangents())
      {
        ezVec3 vTangent = normalsTransform * spFromAssimp(pMesh->mTangents[vertexIndex]);
        vTangent.NormalizeIfNotZero(ezVec3::MakeZero()).IgnoreResult();

        ezVec3 vBitangent = normalsTransform * spFromAssimp(pMesh->mBitangents[vertexIndex]);
        vBitangent.NormalizeIfNotZero(ezVec3::MakeZero()).IgnoreResult();

        if (m_AssimpContext.m_Streams.m_uiTangentStreamIndex != ezInvalidIndex)
        {
          spMeshDataUtils::EncodeTangent(
            vTangent,
            1.0f,
            m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTangentStreamIndex, uiFinalVertIndex),
            GetTangentInputFormat(m_Configuration.m_AssimpImporterConfig.m_eNormalPrecision))
            .IgnoreResult();
        }

        if (m_AssimpContext.m_Streams.m_uiBiTangentStreamIndex != ezInvalidIndex)
        {
          spMeshDataUtils::EncodeTangent(
            vBitangent,
            1.0f,
            m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiBiTangentStreamIndex, uiFinalVertIndex),
            GetTangentInputFormat(m_Configuration.m_AssimpImporterConfig.m_eNormalPrecision))
            .IgnoreResult();
        }
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord0StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(0))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[0][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord0StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord1StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(1))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[1][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord1StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord2StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(2))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[2][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord2StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord3StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(3))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[3][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord3StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord4StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(4))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[4][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord4StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord5StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(5))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[5][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord5StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord6StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(6))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[6][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord6StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiTexCoord7StreamIndex != ezInvalidIndex && pMesh->HasTextureCoords(7))
      {
        const ezVec2 vTexCoord = spFromAssimp(pMesh->mTextureCoords[7][vertexIndex]).GetAsVec2();

        spMeshDataUtils::EncodeTexCoord(
          vTexCoord,
          m_AssimpContext.m_MeshData.GetVertexStreamData(m_AssimpContext.m_Streams.m_uiTexCoord7StreamIndex, uiFinalVertIndex),
          GetTexCoordInputFormat(m_Configuration.m_AssimpImporterConfig.m_eTexCoordPrecision))
          .IgnoreResult();
      }

      if (m_AssimpContext.m_Streams.m_uiColor0StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(0))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[0][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor0StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor1StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(1))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[1][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor1StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor2StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(2))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[2][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor2StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor3StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(3))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[3][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor3StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor4StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(4))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[4][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor4StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor5StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(5))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[5][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor5StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor6StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(6))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[6][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor6StreamIndex, uiFinalVertIndex, color);
      }

      if (m_AssimpContext.m_Streams.m_uiColor7StreamIndex != ezInvalidIndex && pMesh->HasVertexColors(7))
      {
        const ezColorLinearUB color = spFromAssimp(pMesh->mColors[7][vertexIndex]);
        m_AssimpContext.m_MeshData.SetVertexStreamData(m_AssimpContext.m_Streams.m_uiColor7StreamIndex, uiFinalVertIndex, color);
      }
    }

    // Populate the index buffer of the mesh data
    for (ezUInt32 faceIndex = 0; faceIndex < pMesh->mNumFaces; ++faceIndex)
    {
      m_AssimpContext.m_MeshData.SetPrimitiveIndices(
        (mesh.m_uiBaseIndex / 3) + faceIndex,
        pMesh->mFaces[faceIndex].mIndices[0],
        pMesh->mFaces[faceIndex].mIndices[1],
        pMesh->mFaces[faceIndex].mIndices[2]);
    }

    // Perform mesh optimization if required
    OptimizeMeshData(
      m_AssimpContext.m_MeshData.m_Vertices.GetData() + (mesh.m_uiBaseVertex * m_AssimpContext.m_MeshData.m_uiVertexSize),
      mesh.m_uiVertexCount,
      m_AssimpContext.m_MeshData.m_uiVertexSize,
      m_AssimpContext.m_MeshData.m_Indices.GetData() + (mesh.m_uiBaseIndex * sizeof(ezUInt16)),
      mesh.m_uiIndexCount,
      mesh.m_MeshOptRemap);
  }
}

void spMeshProcessor::ProcessBlendShapes()
{
  const ezUInt32 uiMeshCount = m_AssimpContext.m_Meshes.GetCount();

  const auto& ePositionStreamFormat = m_AssimpContext.m_MeshData.GetStreamFormat(m_AssimpContext.m_Streams.m_uiPositionStreamIndex);

  const ezUInt32 uiPositionStreamSize = RHI::spPixelFormatHelper::GetSizeInBytes(ePositionStreamFormat);

  const ezUInt32 uiPositionStreamOffset = 0;

  const ezUInt32 uiBlendShapeVertexSize = uiPositionStreamSize;

  // Read this node mesh data
  for (ezUInt32 uiMeshIndex = 0; uiMeshIndex < uiMeshCount; ++uiMeshIndex)
  {
    const spAssimpMesh& mesh = m_AssimpContext.m_Meshes[uiMeshIndex];
    const spAssimpNode& meshNode = m_AssimpContext.m_Nodes[mesh.m_uiParentNodeIndex];
    const aiMesh* pMesh = mesh.m_pMesh;

    // Process blend shape data
    for (ezUInt32 uiBlendShapeIndex = 0; uiBlendShapeIndex < pMesh->mNumAnimMeshes; ++uiBlendShapeIndex)
    {
      const aiAnimMesh* pAnimMesh = pMesh->mAnimMeshes[uiBlendShapeIndex];

      spAssimpBlendShape& blendShapeNode = m_AssimpContext.m_BlendShapes.ExpandAndGetRef();
      blendShapeNode.m_uiParentMeshIndex = uiMeshIndex;
      blendShapeNode.m_fWeight = pAnimMesh->mWeight;
      blendShapeNode.m_pAnimMesh = pAnimMesh;
      blendShapeNode.m_uiVertexSize = uiBlendShapeVertexSize;

      const ezUInt32 uiVertexCount = pAnimMesh->mNumVertices;

      blendShapeNode.m_BlendShapeData.SetCount(uiVertexCount * uiBlendShapeVertexSize);

      // Populate the vertex buffer of the mesh data
      for (ezUInt32 vertexIndex = 0; vertexIndex < uiVertexCount; ++vertexIndex)
      {
        const ezVec3 vPosition = spFromAssimp(pAnimMesh->mVertices[vertexIndex]);

        spMeshDataUtils::SetVertexStreamData(blendShapeNode.m_BlendShapeData, vertexIndex, uiBlendShapeVertexSize, uiPositionStreamOffset, ePositionStreamFormat, vPosition);
      }

      // Perform mesh optimization if required
      if (m_Configuration.m_AssimpImporterConfig.m_bOptimizeMesh)
      {
        meshopt_remapVertexBuffer(
          blendShapeNode.m_BlendShapeData.GetData(),
          blendShapeNode.m_BlendShapeData.GetData(),
          uiVertexCount,
          uiBlendShapeVertexSize,
          mesh.m_MeshOptRemap.GetData());
      }
    }
  }
}

void spMeshProcessor::OptimizeMeshData(ezUInt8* pVertexBuffer, ezUInt32 uiVertexCount, ezUInt32 uiVertexSize, ezUInt8* pIndexBuffer, ezUInt32 uiIndexCount, ezDynamicArray<ezUInt32>& ref_remap) const
{
  if (!m_Configuration.m_AssimpImporterConfig.m_bOptimizeMesh)
    return;

  ref_remap.SetCountUninitialized(uiVertexCount);

  meshopt_optimizeVertexCache(
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    uiIndexCount,
    uiVertexCount);

  meshopt_optimizeOverdraw(
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    uiIndexCount,
    reinterpret_cast<float*>(pVertexBuffer),
    uiVertexCount,
    uiVertexSize,
    kMeshOptimizationThreshold);

  meshopt_optimizeVertexFetchRemap(
    ref_remap.GetData(),
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    uiIndexCount,
    uiVertexCount);

  meshopt_remapIndexBuffer(
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    reinterpret_cast<ezUInt16*>(pIndexBuffer),
    uiIndexCount,
    ref_remap.GetData());

  meshopt_remapVertexBuffer(
    pVertexBuffer,
    pVertexBuffer,
    uiVertexCount,
    uiVertexSize,
    ref_remap.GetData());
}
