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

#include <RPI/RPIPCH.h>

#include <RPI/Resources/SkeletonResource.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

namespace RPI
{
  static constexpr ezTypeVersion kSkeletonResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSkeletonResource, kSkeletonResourceVersion, ezRTTIDefaultAllocator<spSkeletonResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spSkeletonResource);
  // clang-format on

#pragma region spSkeletonResourceDescriptor

  spSkeletonResourceDescriptor::spSkeletonResourceDescriptor()
  {
    Clear();
  }

  void spSkeletonResourceDescriptor::Clear()
  {
    m_Skeleton.GetJoints().Clear();
  }

  ezResult spSkeletonResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kSkeletonResourceVersion);

    ezUInt8 uiCompressionMode = 0;

    ezStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    uiCompressionMode = 1;
    ezCompressedStreamWriterZstd compressor(&inout_stream, 0, ezCompressedStreamWriterZstd::Compression::Average);
    pWriter = &compressor;
#endif

    inout_stream << uiCompressionMode;

    EZ_SUCCEED_OR_RETURN(pWriter->WriteArray(m_Skeleton.GetJoints()));

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    compressor.FinishCompressedStream().IgnoreResult();
    ezLog::Dev("Compressed skeleton data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

    return EZ_SUCCESS;
  }

  ezResult spSkeletonResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kSkeletonResourceVersion);

    ezUInt8 uiCompressionMode = 0;
    inout_stream >> uiCompressionMode;

    ezStreamReader* pReader = &inout_stream;

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
        pReader = &decompressorZstd;
        break;
#else
        ezLog::Error("Asset is compressed with zstandard, but support for this compressor is not compiled in.");
        return EZ_FAILURE;
#endif

      default:
        ezLog::Error("Asset is compressed with an unknown algorithm.");
        return EZ_FAILURE;
    }

    EZ_SUCCEED_OR_RETURN(pReader->ReadArray(m_Skeleton.GetJoints()));

    return EZ_SUCCESS;
  }

  ezResult spSkeletonResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spSkeletonResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open skeleton asset '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spSkeletonResource

  spSkeletonResource::spSkeletonResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
  }

  ezResourceLoadDesc spSkeletonResource::UnloadData(Unload WhatToUnload)
  {
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 1;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spSkeletonResource::UpdateContent(ezStreamReader* pStream)
  {
    spSkeletonResourceDescriptor desc;

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

  void spSkeletonResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spSkeletonResource) + m_Descriptor.m_Skeleton.GetJoints().GetHeapMemoryUsage();
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spSkeletonResource, spSkeletonResourceDescriptor)
  {
    m_Descriptor = descriptor;

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Loaded;

    return res;
  }

#pragma endregion
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_SkeletonResource);
