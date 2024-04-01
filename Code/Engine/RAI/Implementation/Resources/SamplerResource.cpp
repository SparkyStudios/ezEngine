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

#include <RAI/Resources/SamplerResource.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Utilities/AssetFileHeader.h>

namespace RAI
{
  static constexpr ezTypeVersion kSamplerResourceVersion = 1;

  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSamplerResource, kSamplerResourceVersion, ezRTTIDefaultAllocator<spSamplerResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spSamplerResource);
  // clang-format on

#pragma region spSamplerResourceDescriptor

  spSamplerResourceDescriptor::spSamplerResourceDescriptor()
  {
    Clear();
  }

  void spSamplerResourceDescriptor::Clear()
  {
    m_Sampler.SetDescription(RHI::spSamplerDescription::Linear);
  }

  ezResult spSamplerResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kSamplerResourceVersion);
    inout_stream << m_Sampler; // Write the sampler description

    return EZ_SUCCESS;
  }

  ezResult spSamplerResourceDescriptor::Save(ezStringView sFile)
  {
    EZ_LOG_BLOCK("spSamplerResourceDescriptor::Save", sFile);

    ezFileWriter file;
    if (file.Open(sFile, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open sampler file '{0}'", sFile);
      return EZ_FAILURE;
    }

    return Save(file);
  }

  ezResult spSamplerResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kSamplerResourceVersion);
    inout_stream >> m_Sampler;

    return EZ_SUCCESS;
  }

  ezResult spSamplerResourceDescriptor::Load(ezStringView sFile)
  {
    EZ_LOG_BLOCK("spSamplerResourceDescriptor::Load", sFile);

    ezFileReader file;
    if (file.Open(sFile, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open sampler file '{0}'", sFile);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spSamplerResource

  spSamplerResource::spSamplerResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
  }

  ezResourceLoadDesc spSamplerResource::UnloadData(Unload WhatToUnload)
  {
    if (m_Descriptor.m_Sampler.m_RHISampler != nullptr)
      m_Descriptor.m_Sampler.m_RHISampler.Clear();

    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 1;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spSamplerResource::UpdateContent(ezStreamReader* pStream)
  {
    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;

    if (pStream == nullptr)
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    spSamplerResourceDescriptor desc;

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

  void spSamplerResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spSamplerResource);
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spSamplerResource, spSamplerResourceDescriptor)
  {
    // Create the RHI sampler resource
    descriptor.m_Sampler.CreateRHISampler();

#if EZ_ENABLED(EZ_COMPILE_FOR_DEBUG)
    descriptor.m_Sampler.m_RHISampler->SetDebugName(GetResourceDescription());
#endif

    m_Descriptor = std::move(descriptor);

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 1;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Loaded;

    return res;
  }

#pragma endregion

} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Resources_SamplersResource);
