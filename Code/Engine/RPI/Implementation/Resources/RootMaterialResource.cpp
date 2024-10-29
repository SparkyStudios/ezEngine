// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <RPI/Resources/RootMaterialResource.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

namespace RPI
{
  static constexpr ezTypeVersion kMaterialResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spRootMaterialResource, 1, ezRTTIDefaultAllocator<spRootMaterialResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spRootMaterialResource);
  // clang-format on

#pragma region spRootMaterialResourceDescriptor

  spRootMaterialResourceDescriptor::spRootMaterialResourceDescriptor()
  {
    Clear();
  }

  void spRootMaterialResourceDescriptor::Clear()
  {
    m_RootMaterial.Clear();
  }

  ezResult spRootMaterialResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kMaterialResourceVersion);

    ezUInt8 uiCompressionMode = 0;

    ezStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    uiCompressionMode = 1;
    ezCompressedStreamWriterZstd compressor(&inout_stream, 0, ezCompressedStreamWriterZstd::Compression::Fast);
    pWriter = &compressor;
#endif

    inout_stream << uiCompressionMode;

    // Write root material metadata
    *pWriter << m_RootMaterial.m_Metadata;

    // Write shader data
    const ezUInt64 uiShaderBytesSize = m_RootMaterial.m_ShaderBytes.GetCount();
    EZ_SUCCEED_OR_RETURN(pWriter->WriteQWordValue(&uiShaderBytesSize));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteBytes(m_RootMaterial.m_ShaderBytes.GetPtr(), uiShaderBytesSize));

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    EZ_SUCCEED_OR_RETURN(compressor.FinishCompressedStream());
    ezLog::Dev("Compressed shader data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

    return EZ_SUCCESS;
  }

  ezResult spRootMaterialResourceDescriptor::Save(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spRootMaterialResourceDescriptor::Save", sFileName);

    ezFileWriter file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open material file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Save(file);
  }

  ezResult spRootMaterialResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kMaterialResourceVersion);

    ezUInt8 uiCompressionMode = 0;
    inout_stream >> uiCompressionMode;

    ezStreamReader* pReader = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    ezCompressedStreamReaderZstd decompressor;
#endif

    switch (uiCompressionMode)
    {
      case 0:
        break;

      case 1:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
        decompressor.SetInputStream(&inout_stream);
        pReader = &decompressor;
        break;
#else
        ezLog::Error("Asset is compressed using Zstd, but ZSTD support is not enabled. Please enable it in your build configuration.");
        return EZ_FAILURE;
#endif

      default:
        ezLog::Error("Asset is either corrupted or compressed with an unknown algorithm.");
        return EZ_FAILURE;
    }

    m_RootMaterial.Clear();

    // Read root material metadata
    *pReader >> m_RootMaterial.m_Metadata;

    // Read shader data
    {
      ezUInt64 uiShaderBytesSize = 0;
      EZ_SUCCEED_OR_RETURN(pReader->ReadQWordValue(&uiShaderBytesSize));

      m_RootMaterial.m_ShaderBytes = EZ_DEFAULT_NEW_ARRAY(ezUInt8, uiShaderBytesSize);
      pReader->ReadBytes(m_RootMaterial.m_ShaderBytes.GetPtr(), uiShaderBytesSize);
    }

    return EZ_SUCCESS;
  }

  ezResult spRootMaterialResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spRootMaterialResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open root material resource '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spRootMaterialResource

  ezTypeVersion spRootMaterialResource::GetResourceVersion()
  {
    return kMaterialResourceVersion;
  }

  spRootMaterialResource::spRootMaterialResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
  }

  ezResourceLoadDesc spRootMaterialResource::UnloadData(Unload WhatToUnload)
  {
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spRootMaterialResource::UpdateContent(ezStreamReader* pStream)
  {
    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Unloaded;

    if (pStream == nullptr)
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    spRootMaterialResourceDescriptor desc;

    if (desc.Load(*pStream).Failed())
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    return CreateResource(std::move(desc));
  }

  void spRootMaterialResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spRootMaterialResource) + m_Descriptor.m_RootMaterial.m_Metadata.m_Flags.GetHeapMemoryUsage() + m_Descriptor.m_RootMaterial.m_Metadata.m_Data.GetHeapMemoryUsage() + m_Descriptor.m_RootMaterial.m_Metadata.m_SpecializationConstants.GetHeapMemoryUsage() + m_Descriptor.m_RootMaterial.m_Metadata.m_Properties.GetHeapMemoryUsage() + m_Descriptor.m_RootMaterial.m_ShaderBytes.GetCount();
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spRootMaterialResource, spRootMaterialResourceDescriptor)
  {
    m_Descriptor = std::move(descriptor);

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 1;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Loaded;

    return res;
  }

#pragma endregion
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_RootMaterialResource);
