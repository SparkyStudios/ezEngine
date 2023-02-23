#include <RPI/RPIPCH.h>

#include <RPI/Resources/MeshResource.h>

#include <Core/Assets/AssetFileHeader.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMeshResource, 1, ezRTTIDefaultAllocator<spMeshResource>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spMeshResource);
// clang-format on

#pragma region spMeshResourceDescriptor

static constexpr ezUInt16 kVersion = 1;

static constexpr char kMetadataChunkName[] = "Metadata";
static constexpr char kLODsDataChunkName[] = "LODsData";
static constexpr char kLODsNodeChunkName[] = "LODsNode";

spMeshResourceDescriptor::spMeshResourceDescriptor()
{
  Clear();
}

void spMeshResourceDescriptor::Clear()
{
  m_uiNumLOD = 0;

  m_LOD0.Clear();

  m_Bounds.SetInvalid();
}

const spMesh& spMeshResourceDescriptor::GetLOD(ezUInt32 uiLodIndex) const
{
  switch (uiLodIndex)
  {
    case 0:
      return m_LOD0;
  }

  EZ_ASSERT_DEV(false, "Invalid LOD index {0}", uiLodIndex);
  return m_LOD0;
}

spMesh& spMeshResourceDescriptor::WriteLOD(ezUInt32 uiLodIndex)
{
  switch (uiLodIndex)
  {
    case 0:
      return m_LOD0;
  }

  EZ_ASSERT_DEV(false, "Invalid LOD index {0}", uiLodIndex);
  return m_LOD0;
}

void spMeshResourceDescriptor::ClearLOD(ezUInt32 uiLodIndex)
{
  EZ_ASSERT_DEV(uiLodIndex < SP_RPI_MAX_LOD_COUNT, "Invalid LOD Index. Values are 0 to SP_RPI_MAX_LOD_COUNT.");
  WriteLOD(uiLodIndex).Clear();
}

void spMeshResourceDescriptor::SetLOD(ezUInt32 uiLodIndex, const spMesh& mesh)
{
  EZ_ASSERT_DEV(uiLodIndex < SP_RPI_MAX_LOD_COUNT, "Invalid LOD Index. Values are 0 to SP_RPI_MAX_LOD_COUNT.");
  m_uiNumLOD = uiLodIndex + 1;
  WriteLOD(uiLodIndex) = mesh;
}

ezResult spMeshResourceDescriptor::Save(ezStreamWriter& stream)
{
  stream << kVersion;

  ezUInt8 uiCompressionMode = 0;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  uiCompressionMode = 1;
  ezCompressedStreamWriterZstd compressor(&stream, ezCompressedStreamWriterZstd::Compression::Average);
  ezChunkStreamWriter chunk(compressor);
#else
  ezChunkStreamWriter chunk(stream);
#endif

  stream << uiCompressionMode;

  chunk.BeginStream(1);
  {
    chunk.BeginChunk(kMetadataChunkName, 1);
    {
      // Number of mesh LODs
      chunk << m_uiNumLOD;

      // Mesh bounds
      chunk << m_Bounds;
    }
    chunk.EndChunk();

    chunk.BeginChunk(kLODsDataChunkName, 1);
    {
      WriteMeshData(chunk, m_LOD0);
    }
    chunk.EndChunk();

    chunk.BeginChunk(kLODsNodeChunkName, 1);
    {
      WriteMeshNode(chunk, m_LOD0.m_Root);
    }
    chunk.EndChunk();
  }
  chunk.EndStream();

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  compressor.FinishCompressedStream().IgnoreResult();

  ezLog::Dev("Compressed mesh data from {0} KB to {1} KB ({2}%%)", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

  return EZ_SUCCESS;
}

ezResult spMeshResourceDescriptor::Save(ezStringView sFile)
{
  EZ_LOG_BLOCK("spMeshResourceDescriptor::Save", sFile);

  ezFileWriter file;
  if (file.Open(sFile, 1024 * 1024).Failed())
  {
    ezLog::Error("Failed to open mesh file '{0}'", sFile);
    return EZ_FAILURE;
  }

  return Save(file);
}

ezResult spMeshResourceDescriptor::Load(ezStreamReader& inout_stream)
{
  ezUInt16 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion != kVersion)
    return EZ_FAILURE;

  ezUInt8 uiCompressionMode = 0;
  inout_stream >> uiCompressionMode;

  ezStreamReader* pCompressor = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  ezCompressedStreamReaderZstd decompressorZstd;
#endif

  switch (uiCompressionMode)
  {
    case 0:
      break;

    case 1:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
      decompressorZstd.SetInputStream(&inout_stream);
      pCompressor = &decompressorZstd;
      break;
#else
      ezLog::Error("Mesh is compressed with zstandard, but support for this compressor is not compiled in.");
      return EZ_FAILURE;
#endif

    default:
      ezLog::Error("Mesh is compressed with an unknown algorithm.");
      return EZ_FAILURE;
  }

  ezChunkStreamReader chunk(*pCompressor);

  ezUInt16 uiStreamVersion = chunk.BeginStream();
  {
    while (chunk.GetCurrentChunk().m_bValid)
    {
      const auto& ci = chunk.GetCurrentChunk();

      // Metadata chunk
      if (ci.m_sChunkName == kMetadataChunkName)
      {
        if (ci.m_uiChunkVersion != 1)
        {
          ezLog::Error("Version of chunk '{0}' is invalid ({1})", ci.m_sChunkName, ci.m_uiChunkVersion);
          return EZ_FAILURE;
        }

        // Number of LODs
        chunk >> m_uiNumLOD;

        // Mesh bounds
        chunk >> m_Bounds;
      }

      // LODs Data chunk
      else if (ci.m_sChunkName == kLODsDataChunkName)
      {
        if (ci.m_uiChunkVersion != 1)
        {
          ezLog::Error("Version of chunk '{0}' is invalid ({1})", ci.m_sChunkName, ci.m_uiChunkVersion);
          return EZ_FAILURE;
        }

        ReadMeshData(chunk, m_LOD0);
      }

      // LODs Node chunk
      else if (ci.m_sChunkName == kLODsNodeChunkName)
      {
        if (ci.m_uiChunkVersion != 1)
        {
          ezLog::Error("Version of chunk '{0}' is invalid ({1})", ci.m_sChunkName, ci.m_uiChunkVersion);
          return EZ_FAILURE;
        }

        ReadMeshNode(chunk, m_LOD0.m_Root);
      }

      chunk.NextChunk();
    }
  }
  chunk.EndStream();

  return EZ_SUCCESS;
}

ezResult spMeshResourceDescriptor::Load(ezStringView sFile)
{
  EZ_LOG_BLOCK("spMeshResourceDescriptor::Load", sFile);

  ezFileReader file;
  if (file.Open(sFile, 1024 * 1024).Failed())
  {
    ezLog::Error("Failed to open mesh file '{0}'", sFile);
    return EZ_FAILURE;
  }

  return Load(file);
}

void spMeshResourceDescriptor::WriteMeshData(ezChunkStreamWriter& chunk, const spMesh& mesh)
{
  // Vertex buffer
  {
    const ezUInt32 uiNumVertices = mesh.GetData().m_Vertices.GetCount();

    // Number of vertices
    chunk << uiNumVertices;

    // The vertex buffer data
    for (ezUInt32 i = 0; i < uiNumVertices; ++i)
    {
      chunk << mesh.GetData().m_Vertices[i].m_vPosition;
      chunk << mesh.GetData().m_Vertices[i].m_vNormal;
      chunk << mesh.GetData().m_Vertices[i].m_vTangent;
      chunk << mesh.GetData().m_Vertices[i].m_vBiTangent;
      chunk << mesh.GetData().m_Vertices[i].m_vTexCoord0;
      chunk << mesh.GetData().m_Vertices[i].m_vTexCoord1;
      chunk << mesh.GetData().m_Vertices[i].m_Color0;
      chunk << mesh.GetData().m_Vertices[i].m_Color1;
    }
  }

  // Index buffer
  {
    // The index buffer data
    chunk.WriteArray(mesh.GetData().m_Indices).IgnoreResult();
  }
}

void spMeshResourceDescriptor::ReadMeshData(ezChunkStreamReader& chunk, spMesh& mesh)
{
  // Vertices
  ezUInt32 uiNumVertices = 0;
  chunk >> uiNumVertices;
  mesh.m_Data.m_Vertices.SetCount(uiNumVertices);

  for (ezUInt32 v = 0; v < uiNumVertices; ++v)
  {
    chunk >> mesh.m_Data.m_Vertices[v].m_vPosition;
    chunk >> mesh.m_Data.m_Vertices[v].m_vNormal;
    chunk >> mesh.m_Data.m_Vertices[v].m_vTangent;
    chunk >> mesh.m_Data.m_Vertices[v].m_vBiTangent;
    chunk >> mesh.m_Data.m_Vertices[v].m_vTexCoord0;
    chunk >> mesh.m_Data.m_Vertices[v].m_vTexCoord1;
    chunk >> mesh.m_Data.m_Vertices[v].m_Color0;
    chunk >> mesh.m_Data.m_Vertices[v].m_Color1;
  }

  // Indices
  chunk.ReadArray(mesh.m_Data.m_Indices).IgnoreResult();
}

void spMeshResourceDescriptor::WriteMeshNode(ezChunkStreamWriter& chunk, const spMesh::Node& node)
{
  // Node name
  chunk << node.m_sName;

  // Node transform
  chunk << node.m_Transform.m_vPosition;
  chunk << node.m_Transform.m_vScale;
  chunk << node.m_Transform.m_vRotation;

  // Node material
  chunk << node.m_sMaterial;

  // Node meshes
  chunk << node.m_Entries.GetCount();
  for (const auto& entry : node.m_Entries)
  {
    chunk << entry.m_sName;
    chunk << entry.m_uiBaseIndex;
    chunk << entry.m_uiIndicesCount;
    chunk << entry.m_uiBaseVertex;
    chunk << entry.m_uiVerticesCount;
  }

  // Node children
  chunk << node.m_Children.GetCount();
  for (const auto& child : node.m_Children)
  {
    WriteMeshNode(chunk, child);
  }
}

void spMeshResourceDescriptor::ReadMeshNode(ezChunkStreamReader& chunk, spMesh::Node& node)
{
  // Node name
  chunk >> node.m_sName;

  // Node transform
  chunk >> node.m_Transform.m_vPosition;
  chunk >> node.m_Transform.m_vScale;
  chunk >> node.m_Transform.m_vRotation;

  // Node material
  chunk >> node.m_sMaterial;

  // Node meshes
  ezUInt32 uiEntryCount = 0;
  chunk >> uiEntryCount;
  node.m_Entries.SetCount(uiEntryCount);
  for (ezUInt32 i = 0; i < uiEntryCount; ++i)
  {
    chunk >> node.m_Entries[i].m_sName;
    chunk >> node.m_Entries[i].m_uiBaseIndex;
    chunk >> node.m_Entries[i].m_uiIndicesCount;
    chunk >> node.m_Entries[i].m_uiBaseVertex;
    chunk >> node.m_Entries[i].m_uiVerticesCount;
  }

  // Node children
  ezUInt32 uiChildCount = 0;
  chunk >> uiChildCount;
  node.m_Children.SetCount(uiChildCount);
  for (ezUInt32 i = 0; i < uiChildCount; ++i)
  {
    ReadMeshNode(chunk, node.m_Children[i]);
  }
}

#pragma endregion

#pragma region spMeshResource

spMeshResource::spMeshResource()
  : ezResource(DoUpdate::OnAnyThread, SP_RPI_MAX_LOD_COUNT)
{
  m_Bounds.SetInvalid();
}

ezResourceLoadDesc spMeshResource::UnloadData(Unload WhatToUnload)
{
  ezUInt8 uiNumLODs = m_Descriptor.GetNumLODs();

  if (uiNumLODs > 0)
  {
    for (ezUInt32 i = 0, l = uiNumLODs; i < l; ++i)
    {
      m_Descriptor.ClearLOD(i);
      --uiNumLODs;

      if (WhatToUnload == Unload::OneQualityLevel || uiNumLODs == 0)
        break;
    }
  }

  if (WhatToUnload == Unload::AllQualityLevels)
  {
    m_Descriptor.Clear();
  }

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = uiNumLODs;
  res.m_uiQualityLevelsLoadable = SP_RPI_MAX_LOD_COUNT - uiNumLODs;
  res.m_State = uiNumLODs == 0 ? ezResourceState::Unloaded : ezResourceState::Loaded;

  return res;
}

ezResourceLoadDesc spMeshResource::UpdateContent(ezStreamReader* pStream)
{
  spMeshResourceDescriptor desc;

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (pStream == nullptr)
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    ezStringBuilder sAbsFilePath;
    *pStream >> sAbsFilePath;
  }

  ezAssetFileHeader AssetHash;
  AssetHash.Read(*pStream).IgnoreResult();

  if (desc.Load(*pStream).Failed())
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  return CreateResource(std::move(desc));
}

void spMeshResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  // TODO
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spMeshResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

EZ_RESOURCE_IMPLEMENT_CREATEABLE(spMeshResource, spMeshResourceDescriptor)
{
  m_Descriptor = descriptor;

  m_Bounds = descriptor.m_Bounds;
  EZ_ASSERT_DEV(m_Bounds.IsValid(), "The mesh bounds are invalid. Make sure to call ezMeshResourceDescriptor::ComputeBounds()");

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Loaded;

  return res;
}

#pragma endregion

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_MeshResource);
