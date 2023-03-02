#include <RAI/RAIPCH.h>

#include <RAI/Resources/MeshResource.h>

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

  m_LODs.Reserve(SP_RAI_MAX_LOD_COUNT);

  m_Bounds.SetInvalid();
}

const spMesh& spMeshResourceDescriptor::GetLOD(ezUInt32 uiLodIndex) const
{
  EZ_ASSERT_DEV(uiLodIndex < SP_RAI_MAX_LOD_COUNT, "Invalid LOD index {0}. Value is greater than the maximum number of LODs.", uiLodIndex);
  EZ_ASSERT_DEV(uiLodIndex < m_LODs.GetCount(), "Invalid LOD index {0}. Value is greater than the available number of LODs.", uiLodIndex);

  return m_LODs[uiLodIndex];
}

spMesh& spMeshResourceDescriptor::WriteLOD(ezUInt32 uiLodIndex)
{
  EZ_ASSERT_DEV(uiLodIndex < SP_RAI_MAX_LOD_COUNT, "Invalid LOD index {0}. Value is greater than the maximum number of LODs.", uiLodIndex);
  EZ_ASSERT_DEV(uiLodIndex < m_LODs.GetCount(), "Invalid LOD index {0}. Value is greater than the available number of LODs.", uiLodIndex);

  return m_LODs[uiLodIndex];
}

void spMeshResourceDescriptor::ClearLOD(ezUInt32 uiLodIndex)
{
  EZ_ASSERT_DEV(uiLodIndex < SP_RAI_MAX_LOD_COUNT, "Invalid LOD index {0}. Value is greater than the maximum number of LODs.", uiLodIndex);
  EZ_ASSERT_DEV(uiLodIndex < m_LODs.GetCount(), "Invalid LOD index {0}. Value is greater than the available number of LODs.", uiLodIndex);

  WriteLOD(uiLodIndex).Clear();
}

void spMeshResourceDescriptor::SetLOD(ezUInt32 uiLodIndex, const spMesh& mesh)
{
  EZ_ASSERT_DEV(uiLodIndex < SP_RAI_MAX_LOD_COUNT, "Invalid LOD index {0}. Value is greater than the maximum number of LODs.", uiLodIndex);

  if (m_uiNumLOD <= uiLodIndex)
  {
    m_uiNumLOD = uiLodIndex + 1;
    m_LODs.EnsureCount(m_uiNumLOD);
  }

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
      for (auto& lod : m_LODs)
        chunk << lod.m_Data;
    }
    chunk.EndChunk();

    chunk.BeginChunk(kLODsNodeChunkName, 1);
    {
      for (auto& lod : m_LODs)
        chunk << lod.m_Root;
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

        m_LODs.SetCount(m_uiNumLOD);

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

        for (auto& lod : m_LODs)
          chunk >> lod.m_Data;
      }

      // LODs Node chunk
      else if (ci.m_sChunkName == kLODsNodeChunkName)
      {
        if (ci.m_uiChunkVersion != 1)
        {
          ezLog::Error("Version of chunk '{0}' is invalid ({1})", ci.m_sChunkName, ci.m_uiChunkVersion);
          return EZ_FAILURE;
        }

        for (auto& lod : m_LODs)
          chunk >> lod.m_Root;
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

#pragma endregion

#pragma region spMeshResource

spMeshResource::spMeshResource()
  : ezResource(DoUpdate::OnAnyThread, SP_RAI_MAX_LOD_COUNT)
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
  res.m_uiQualityLevelsLoadable = SP_RAI_MAX_LOD_COUNT - uiNumLODs;
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

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Resources_MeshResource);
