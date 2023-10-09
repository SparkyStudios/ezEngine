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

#include <RAI/RAIPCH.h>

#include <RAI/Resources/MeshResource.h>

#include <Core/Assets/AssetFileHeader.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/FileSystem/FileReader.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

namespace RAI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMeshResource, 1, ezRTTIDefaultAllocator<spMeshResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spMeshResource);
  // clang-format on

#pragma region spMeshResourceDescriptor

  static constexpr ezTypeVersion kMeshResourceVersion = 1;

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
    m_LODs.Clear();
    m_LODs.Reserve(SP_RAI_MAX_LOD_COUNT);
    m_Bounds = ezBoundingBoxSphere::MakeInvalid();
  }

  const spMesh& spMeshResourceDescriptor::GetLOD(ezUInt32 uiLodIndex) const
  {
    EZ_ASSERT_DEV(uiLodIndex < SP_RAI_MAX_LOD_COUNT, "Invalid LOD index {0}. Value is greater than the maximum number of LODs.", uiLodIndex);
    EZ_ASSERT_DEV(uiLodIndex < m_LODs.GetCount(), "Invalid LOD index {0}. Value is greater than the available number of LODs.", uiLodIndex);

    return m_LODs[uiLodIndex];
  }

  spMesh& spMeshResourceDescriptor::GetLOD(ezUInt32 uiLodIndex)
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

  ezResult spMeshResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream << kMeshResourceVersion;

    ezUInt8 uiCompressionMode;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    uiCompressionMode = 1;
    ezCompressedStreamWriterZstd compressor(&inout_stream, 0, ezCompressedStreamWriterZstd::Compression::Average);
    ezChunkStreamWriter chunk(compressor);
#endif

    inout_stream << uiCompressionMode;

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
          lod.WriteData(chunk);
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
    ezLog::Dev("Compressed mesh data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

    return EZ_SUCCESS;
  }

  ezResult spMeshResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    ezTypeVersion uiVersion = 0;
    inout_stream >> uiVersion;

    if (uiVersion == 0 || uiVersion > kMeshResourceVersion)
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
          if (ci.m_uiChunkVersion > 1)
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
          if (ci.m_uiChunkVersion > 1)
          {
            ezLog::Error("Version of chunk '{0}' is invalid ({1})", ci.m_sChunkName, ci.m_uiChunkVersion);
            return EZ_FAILURE;
          }

          for (auto& lod : m_LODs)
            lod.ReadData(chunk);
        }

        // LODs Node chunk
        else if (ci.m_sChunkName == kLODsNodeChunkName)
        {
          if (ci.m_uiChunkVersion > 1)
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

  ezResult spMeshResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spMeshResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open mesh file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    // Skip asset header
    ezAssetFileHeader assetHeader;
    EZ_SUCCEED_OR_RETURN(assetHeader.Read(file));

    return Load(file);
  }

#pragma endregion

#pragma region spMeshResource

  spMeshResource::spMeshResource()
    : ezResource(DoUpdate::OnAnyThread, SP_RAI_MAX_LOD_COUNT)
  {
    m_Bounds = ezBoundingBoxSphere::MakeInvalid();
  }

  ezResourceLoadDesc spMeshResource::UnloadData(Unload WhatToUnload)
  {
    const ezUInt8 uiNumLODs = m_Descriptor.GetNumLODs();
    ezUInt8 remainingLODs = uiNumLODs;

    if (uiNumLODs > 0)
    {
      for (ezUInt8 i = 0, l = uiNumLODs; i < l; ++i)
      {
        remainingLODs = uiNumLODs - i - 1;
        m_Descriptor.ClearLOD(remainingLODs);

        if (WhatToUnload == Unload::OneQualityLevel || remainingLODs == 0)
          break;
      }
    }

    if (WhatToUnload == Unload::AllQualityLevels)
    {
      m_Descriptor.Clear();
    }

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = remainingLODs;
    res.m_uiQualityLevelsLoadable = uiNumLODs;
    res.m_State = remainingLODs == 0 ? ezResourceState::Unloaded : ezResourceState::Loaded;

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
    // EZ_ASSERT_DEV(m_Bounds.IsValid(), "The mesh bounds are invalid. Make sure to call ezMeshResourceDescriptor::ComputeBounds()");

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Loaded;

    return res;
  }

#pragma endregion
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Resources_MeshResource);
