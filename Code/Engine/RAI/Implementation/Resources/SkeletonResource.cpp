#include <RAI/RAIPCH.h>

#include <RAI/Resources/SkeletonResource.h>

#ifdef BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT
#  include <Foundation/IO/CompressedStreamBrotliG.h>
#endif

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

namespace RAI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSkeletonResource, 1, ezRTTIDefaultAllocator<spSkeletonResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spSkeletonResource);
  // clang-format on

#pragma region spSkeletonResourceDescriptor

  static constexpr ezTypeVersion kSkeletonResourceVersion = 1;

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
    ezCompressedStreamWriterZstd compressor(&inout_stream, ezCompressedStreamWriterZstd::Compression::Average);
    pWriter = &compressor;
#elif BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT
    uiCompressionMode = 2;
    ezCompressedStreamWriterBrotliG compressor(&inout_stream);
    pWriter = &compressor;
#endif

    inout_stream << uiCompressionMode;

    EZ_SUCCEED_OR_RETURN(pWriter->WriteArray(m_Skeleton.GetJoints()));

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    compressor.FinishCompressedStream().IgnoreResult();
    ezLog::Dev("Compressed skeleton data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#elif BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT
    compressor.FinishCompressedStream().IgnoreResult();
    ezLog::Dev("Compressed skeleton data from {0}KB to {1}KB ({2}%%) using BrotliG", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
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

#ifdef BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT
    ezCompressedStreamReaderBrotliG decompressorBrotliG;
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

      case 2:
#ifdef BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT
        decompressorBrotliG.SetInputStream(&inout_stream);
        pReader = &decompressorBrotliG;
        break;
#else
      ezLog::Error("Asset is compressed with BrotliG, but support for this compressor is not compiled in.");
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

#pragma  region spSkeletonResource

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
} // namespace RAI
