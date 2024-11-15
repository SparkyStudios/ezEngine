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

#include <RPI/Resources/MaterialResource.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

namespace RPI
{
  static constexpr ezTypeVersion kMaterialResourceVersion = 1;

  // clang-format off
  typedef ezRTTIDefaultAllocator<spMaterialResource, ezAlignedAllocatorWrapper> spMaterialResourceAllocator;
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spMaterialResource, 1, spMaterialResourceAllocator)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spMaterialResource);
  // clang-format on

#pragma region spMaterialResourceDescriptor

  spMaterialResourceDescriptor::spMaterialResourceDescriptor()
  {
    Clear();
  }

  void spMaterialResourceDescriptor::Clear()
  {
    m_Material.Clear();
  }


  ezResult spMaterialResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream.WriteVersion(kMaterialResourceVersion);

    inout_stream << m_Material.m_hRootMaterialResource;

    inout_stream << m_Material.m_Data;

    // inout_stream.WriteArray(m_Material.m_SpecializationConstants).AssertSuccess();
    // inout_stream.WriteArray(m_Material.m_Properties).AssertSuccess();

    return EZ_SUCCESS;
  }

  ezResult spMaterialResourceDescriptor::Save(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spMaterialResourceDescriptor::Save", sFileName);

    ezFileWriter file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open material file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Save(file);
  }

  ezResult spMaterialResourceDescriptor::Load(ezStreamReader& inout_stream)
  {
    inout_stream.ReadVersion(kMaterialResourceVersion);

    inout_stream >> m_Material.m_hRootMaterialResource;

    inout_stream >> m_Material.m_Data;

    // inout_stream.ReadArray(m_Material.m_SpecializationConstants).AssertSuccess();
    // inout_stream.ReadArray(m_Material.m_Properties).AssertSuccess();

    return EZ_SUCCESS;
  }

  ezResult spMaterialResourceDescriptor::Load(ezStringView sFileName)
  {
    EZ_LOG_BLOCK("spMaterialResourceDescriptor::Load", sFileName);

    ezFileReader file;
    if (file.Open(sFileName, 1024 * 1024).Failed())
    {
      ezLog::Error("Failed to open material file '{0}'", sFileName);
      return EZ_FAILURE;
    }

    return Load(file);
  }

#pragma endregion

#pragma region spMaterialResource

  ezTypeVersion spMaterialResource::GetResourceVersion()
  {
    return kMaterialResourceVersion;
  }

  spMaterialResource::spMaterialResource()
    : ezResource(DoUpdate::OnAnyThread, 1)
  {
  }

  ezResourceLoadDesc spMaterialResource::UnloadData(Unload WhatToUnload)
  {
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spMaterialResource::UpdateContent(ezStreamReader* pStream)
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

    spMaterialResourceDescriptor desc;

    if (desc.Load(*pStream).Failed())
    {
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    return CreateResource(std::move(desc));
  }

  void spMaterialResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spMaterialResource) + m_Descriptor.m_Material.GetHeapMemoryUsage();
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spMaterialResource, spMaterialResourceDescriptor)
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
