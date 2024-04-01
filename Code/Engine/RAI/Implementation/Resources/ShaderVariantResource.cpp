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

#include <RAI/Resources/ShaderVariantResource.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

namespace RAI
{
  static constexpr ezTypeVersion kShaderVariantResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spShaderVariantResource, 1, ezRTTIDefaultAllocator<spShaderVariantResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spShaderVariantResource);
  // clang-format on

#pragma region spShaderResourceDescriptor

  spShaderVariantResourceDescriptor::spShaderVariantResourceDescriptor()
  {
    Clear();
  }

  void spShaderVariantResourceDescriptor::Clear()
  {
    m_ShaderVariant.Clear();
  }

  ezResult spShaderVariantResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kShaderVariantResourceVersion);

    ezUInt8 uiCompressionMode = 0;

    ezStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    uiCompressionMode = 1;
    ezCompressedStreamWriterZstd compressor(&inout_stream, 0, ezCompressedStreamWriterZstd::Compression::Fast);
    pWriter = &compressor;
#endif

    inout_stream << uiCompressionMode;

    EZ_SUCCEED_OR_RETURN(pWriter->WriteString(m_ShaderVariant.m_sName));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteArray(m_ShaderVariant.m_InputElements));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteArray(m_ShaderVariant.m_Buffer));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteMap(m_ShaderVariant.m_EntryPoints));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteArray(m_ShaderVariant.m_Permutations));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteBytes(&m_ShaderVariant.m_RenderingState, sizeof(RHI::spRenderingState)));
    EZ_SUCCEED_OR_RETURN(pWriter->WriteBytes(&m_ShaderVariant.m_eShaderLanguage, sizeof(RHI::spShaderLanguage)));

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    compressor.FinishCompressedStream().IgnoreResult();
    ezLog::Dev("Compressed shader data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

    return EZ_SUCCESS;
  }

  ezResult spShaderVariantResourceDescriptor::Save(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spShaderVariantResourceDescriptor::Save", sFileName);

    ezFileWriter file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open shader file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Save(file);
  }

  ezResult spShaderVariantResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kShaderVariantResourceVersion);

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

    {
      ezStringBuilder sb;
      EZ_SUCCEED_OR_RETURN(pReader->ReadString(sb));
      m_ShaderVariant.m_sName.Assign(sb);
    }

    EZ_SUCCEED_OR_RETURN(pReader->ReadArray(m_ShaderVariant.m_InputElements));
    EZ_SUCCEED_OR_RETURN(pReader->ReadArray(m_ShaderVariant.m_Buffer));
    EZ_SUCCEED_OR_RETURN(pReader->ReadMap(m_ShaderVariant.m_EntryPoints));
    EZ_SUCCEED_OR_RETURN(pReader->ReadArray(m_ShaderVariant.m_Permutations));
    EZ_SUCCEED_OR_RETURN(pReader->ReadBytes(&m_ShaderVariant.m_RenderingState, sizeof(RHI::spRenderingState)) == sizeof(RHI::spRenderingState) ? EZ_SUCCESS : EZ_FAILURE);
    EZ_SUCCEED_OR_RETURN(pReader->ReadBytes(&m_ShaderVariant.m_eShaderLanguage, sizeof(RHI::spShaderLanguage)) == sizeof(RHI::spShaderLanguage) ? EZ_SUCCESS : EZ_FAILURE);

    return EZ_SUCCESS;
  }

  ezResult spShaderVariantResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spShaderVariantResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open shader file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spShaderVariantResource

  spShaderVariantResource::spShaderVariantResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
  }

  ezResourceLoadDesc spShaderVariantResource::UnloadData(ezResource::Unload WhatToUnload)
  {
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 1;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spShaderVariantResource::UpdateContent(ezStreamReader* pStream)
  {
    spShaderVariantResourceDescriptor desc;

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

  void spShaderVariantResource::UpdateMemoryUsage(ezResource::MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spShaderVariantResource);
    out_NewMemoryUsage.m_uiMemoryGPU = ModifyMemoryUsage().m_uiMemoryGPU;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spShaderVariantResource, spShaderVariantResourceDescriptor)
  {
    m_Descriptor = std::move(descriptor);

    ezUInt32 uiGPUMemoryUsage = 0;
    ModifyMemoryUsage().m_uiMemoryGPU = uiGPUMemoryUsage;

    ezResourceLoadDesc res;

    res.m_State = ezResourceState::Invalid;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;

    // Create the RHI shader program resource
    m_Descriptor.m_ShaderVariant.CreateRHIShaderProgram();

    if (m_Descriptor.m_ShaderVariant.m_pRHIShaderProgram == nullptr)
      return res;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    m_Descriptor.m_ShaderVariant.m_pRHIShaderProgram->SetDebugName(GetResourceDescription());
#endif

    res.m_State = ezResourceState::Loaded;

    // TODO: Get the amount of consumed GPU memory from the shader program.

    return res;
  }

#pragma endregion
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Resources_ShaderVariantResource);
