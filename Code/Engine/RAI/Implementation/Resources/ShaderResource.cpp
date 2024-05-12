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

#include <RAI/Resources/ShaderResource.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Utilities/AssetFileHeader.h>

namespace RAI
{
  static constexpr ezTypeVersion kShaderResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderResource, 1, ezRTTIDefaultAllocator<spShaderResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spShaderResource);
  // clang-format on

#pragma region spShaderResourceDescriptor

  spShaderResourceDescriptor::spShaderResourceDescriptor()
  {
    Clear();
  }

  void spShaderResourceDescriptor::Clear()
  {
    m_Shader.Clear();
  }

  ezResult spShaderResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kShaderResourceVersion);

    ezUInt8 uiCompressionMode = 0;

    ezStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    uiCompressionMode = 1;
    ezCompressedStreamWriterZstd compressor(&inout_stream, 0, ezCompressedStreamWriterZstd::Compression::Fast);
    pWriter = &compressor;
#endif

    inout_stream << uiCompressionMode;

    const ezUInt64 uiShaderBytesSize = m_Shader.m_ShaderBytes.GetCount();
    EZ_SUCCEED_OR_RETURN(pWriter->WriteQWordValue(&uiShaderBytesSize));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteBytes(m_Shader.m_ShaderBytes.GetPtr(), uiShaderBytesSize));

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    EZ_SUCCEED_OR_RETURN(compressor.FinishCompressedStream());
    ezLog::Dev("Compressed shader data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

    return EZ_SUCCESS;
  }

  ezResult spShaderResourceDescriptor::Save(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spShaderResourceDescriptor::Save", sFileName);

    ezFileWriter file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open shader file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Save(file);
  }

  ezResult spShaderResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kShaderResourceVersion);

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
        ezLog::Error("Asset is compressed with zstandard, but support for this compressor is not compiled in.");
        return EZ_FAILURE;
#endif

      default:
        ezLog::Error("Asset is compressed with an unknown algorithm.");
        return EZ_FAILURE;
    }

    ezUInt64 uiShaderBytesSize = 0;
    EZ_SUCCEED_OR_RETURN(pReader->ReadQWordValue(&uiShaderBytesSize));

    m_Shader.m_ShaderBytes = EZ_DEFAULT_NEW_ARRAY(ezUInt8, uiShaderBytesSize);
    pReader->ReadBytes(m_Shader.m_ShaderBytes.GetPtr(), uiShaderBytesSize);

    return EZ_SUCCESS;
  }

  ezResult spShaderResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spShaderResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open shader file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spShaderResource

  ezTypeVersion spShaderResource::GetResourceVersion()
  {
    return kShaderResourceVersion;
  }

  spShaderResource::spShaderResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
  }

  ezResourceLoadDesc spShaderResource::UnloadData(ezResource::Unload WhatToUnload)
  {
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spShaderResource::UpdateContent(ezStreamReader* pStream)
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
    spShaderResourceDescriptor desc;

    if (desc.Load(*pStream).Failed())
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    return CreateResource(std::move(desc));
  }

  void spShaderResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spShaderResource) + m_Descriptor.m_Shader.m_ShaderBytes.GetCount();
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spShaderResource, spShaderResourceDescriptor)
  {
    m_Descriptor = std::move(descriptor);

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 1;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Loaded;

    return res;
  }

#pragma endregion
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Resources_ShaderResource);
