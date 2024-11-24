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

#include <RPI/Resources/BlendShapeResource.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Utilities/AssetFileHeader.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

namespace RPI
{
  static constexpr ezTypeVersion kBlendShapeResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spBlendShapeResource, 1, ezRTTIDefaultAllocator<spBlendShapeResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spBlendShapeResource);
  // clang-format on

#pragma region spBlendShapeResourceDescriptor

  spBlendShapeResourceDescriptor::spBlendShapeResourceDescriptor()
  {
    Clear();
  }

  void spBlendShapeResourceDescriptor::Clear()
  {
    m_BlendShapes.Clear();
  }

  ezArrayPtr<const spBlendShape> spBlendShapeResourceDescriptor::GetBlendShapes(ezStringView sMeshName) const
  {
    ezDynamicArray<spBlendShape> blendShapes;

    for (const auto& pair : m_BlendShapes)
      if (pair.key == ezTempHashedString(sMeshName))
        blendShapes.PushBack(pair.value);

    return blendShapes;
  }

  bool spBlendShapeResourceDescriptor::GetBlendShape(ezStringView sMeshName, ezStringView sName, const spBlendShape*& out_blendShape) const
  {
    for (const auto& pair : m_BlendShapes)
    {
      if (pair.key == ezTempHashedString(sMeshName))
      {
        if (pair.value.m_sName == ezTempHashedString(sName))
        {
          out_blendShape = &pair.value;
          return true;
        }
      }
    }

    return false;
  }

  void spBlendShapeResourceDescriptor::AddBlendShape(ezStringView sMeshName, const spBlendShape& blendShape)
  {
    ezHashedString sHashMeshName;
    sHashMeshName.Assign(sMeshName);
    m_BlendShapes.Insert(sHashMeshName, blendShape);
  }

  ezResult spBlendShapeResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kBlendShapeResourceVersion);
    ezUInt8 uiCompressionMode = 0;

    ezStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    uiCompressionMode = 1;
    ezCompressedStreamWriterZstd compressor(&inout_stream, 0, ezCompressedStreamWriterZstd::Compression::Average);
    pWriter = &compressor;
#endif

    inout_stream << uiCompressionMode;

    *pWriter << m_BlendShapes.GetCount();

    for (const auto& pair : m_BlendShapes)
    {
      *pWriter << pair.key;
      *pWriter << pair.value;
    }

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    compressor.FinishCompressedStream().IgnoreResult();
    ezLog::Dev("Compressed blend shape data from {0}KB to {1}KB ({2}%%) using Zstd", ezArgF(static_cast<double>(compressor.GetUncompressedSize()) / 1024.0, 1), ezArgF(static_cast<double>(compressor.GetCompressedSize()) / 1024.0, 1), ezArgF(100.0 * static_cast<double>(compressor.GetCompressedSize()) / static_cast<double>(compressor.GetUncompressedSize()), 1));
#endif

    return EZ_SUCCESS;
  }

  ezResult spBlendShapeResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kBlendShapeResourceVersion);

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

    ezUInt32 uiCount = 0;
    *pReader >> uiCount;

    m_BlendShapes.Clear();
    auto& data = m_BlendShapes.GetData();
    data.SetCount(uiCount);

    for (ezUInt32 i = 0; i < uiCount; i++)
    {
      *pReader >> data[i].key;
      *pReader >> data[i].value;
    }

    return EZ_SUCCESS;
  }

  ezResult spBlendShapeResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spBlendShapeResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open blend shape file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    ezAssetFileHeader header;
    EZ_SUCCEED_OR_RETURN(header.Read(file));

    return Load(file);
  }

#pragma endregion

#pragma region spBlendShapeResource

  spBlendShapeResource::spBlendShapeResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
    m_Descriptor.Clear();
  }

  ezResourceLoadDesc spBlendShapeResource::UnloadData(Unload WhatToUnload)
  {
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 1;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spBlendShapeResource::UpdateContent(ezStreamReader* pStream)
  {
    spBlendShapeResourceDescriptor desc;

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

  void spBlendShapeResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spBlendShapeResource) + m_Descriptor.m_BlendShapes.GetHeapMemoryUsage();
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spBlendShapeResource, spBlendShapeResourceDescriptor)
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

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_BlendShapeResource);
