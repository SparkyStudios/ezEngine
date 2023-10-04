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

#include <Core/Assets/AssetFileHeader.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/FileSystem/FileReader.h>

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
    inout_stream << kShaderResourceVersion;

    inout_stream << m_Shader.m_sName;
    return inout_stream.WriteArray(m_Shader.m_Variants);
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

    ezStringBuilder sb;
    inout_stream >> sb;
    m_Shader.m_sName = sb;

    m_Shader.m_Variants.Clear();
    return inout_stream.ReadArray(m_Shader.m_Variants);
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

    // skip the absolute file path data that the standard file reader writes into the stream
    {
      ezStringBuilder sAbsFilePath;
      *pStream >> sAbsFilePath;
    }

    spShaderResourceDescriptor desc;

    ezAssetFileHeader AssetHash;
    AssetHash.Read(*pStream).IgnoreResult();

    if (desc.Load(*pStream).Failed())
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    return CreateResource(std::move(desc));
  }

  void spShaderResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spShaderResource) + m_Descriptor.m_Shader.m_Variants.GetHeapMemoryUsage();
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
