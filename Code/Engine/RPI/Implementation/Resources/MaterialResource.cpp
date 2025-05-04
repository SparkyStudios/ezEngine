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

#include <RPI/Materials/MaterialFunctor.h>
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

    const ezResourceLock rootMaterialResource(descriptor.GetRootMaterialResource(), ezResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (!rootMaterialResource.IsValid())
    {
      ezLog::Error("Unable to get the root material resource for material {0}!", GetResourceID());
      res.m_State = ezResourceState::LoadedResourceMissing;
      return res;
    }

    auto& material = descriptor.GetMaterial();
    const auto& rootMaterial = rootMaterialResource.GetPointerNonConst()->GetDescriptor().GetRootMaterial();

    const auto& metadata = rootMaterial.GetMetadata();
    auto& data = material.GetData();

    // Update Material Data
    {
      for (auto it = metadata.m_Data.GetIterator(); it.IsValid(); it.Next())
      {
        ezVariant value = it.Value();

        if (value.IsA<spMaterialFunctorEvaluator>())
        {
          const auto& evaluator = value.Get<spMaterialFunctorEvaluator>();
          value = evaluator(&material);
        }

        if (!value.IsValid())
        {
          ezLog::Warning("Invalid material data value: {0}", it.Key());
          continue;
        }

        if (const auto& key = it.Key(); key == "AlbedoColor")
        {
          data.m_AlbedoColor = value.ConvertTo<ezColor>();
        }
        else if (key == "SpecularColor")
        {
          data.m_SpecularColor = value.ConvertTo<ezColor>();
        }
        else if (key == "EmissiveColor")
        {
          data.m_EmissiveColor = value.ConvertTo<ezColor>();
        }
        else if (key == "UVTiling")
        {
          data.m_UVTiling = value.ConvertTo<ezVec2>();
        }
        else if (key == "UVOffset")
        {
          data.m_UVOffset = value.ConvertTo<ezVec2>();
        }
        else if (key == "Roughness")
        {
          data.m_Roughness = value.ConvertTo<float>();
        }
        else if (key == "Metalness")
        {
          data.m_Metalness = value.ConvertTo<float>();
        }
        else if (key == "NormalIntensity")
        {
          data.m_NormalIntensity = value.ConvertTo<float>();
        }
        else if (key == "Height")
        {
          data.m_Height = value.ConvertTo<float>();
        }
        else if (key == "WorldSpaceHeight")
        {
          data.m_WorldSpaceHeight = value.ConvertTo<bool>();
        }
        else if (key == "IOR")
        {
          data.m_IOR = value.ConvertTo<float>();
        }
        else if (key == "SubsurfaceScattering")
        {
          data.m_SubsurfaceScattering = value.ConvertTo<float>();
        }
        else if (key == "SheenTint")
        {
          data.m_SheenTint = value.ConvertTo<ezColor>().GetAsVec4().GetAsVec3();
        }
        else if (key == "Sheen")
        {
          data.m_Sheen = value.ConvertTo<float>();
        }
        else if (key == "Anisotropic")
        {
          data.m_Anisotropic = value.ConvertTo<float>();
        }
        else if (key == "AnisotropicRotation")
        {
          data.m_AnisotropicRotation = value.ConvertTo<float>();
        }
        else if (key == "Clearcoat")
        {
          data.m_Clearcoat = value.ConvertTo<float>();
        }
        else if (key == "ClearcoatRoughness")
        {
          data.m_ClearcoatRoughness = value.ConvertTo<float>();
        }
        else
        {
          ezLog::Error("Unsupported material data: {0}", it.Key());
        }
      }
    }

    // Update material Flags
    {
      ezUInt32 flags = 0;

      for (auto it = metadata.m_Flags.GetIterator(); it.IsValid(); it.Next())
      {
        ezVariant value = it.Value();

        if (it.Value().IsA<spMaterialFunctorEvaluator>())
        {
          ezLog::Info("Updating material parameter: {0}", it.Key());
          const auto& evaluator = it.Value().Get<spMaterialFunctorEvaluator>();
          value = evaluator(&material);
        }

        if (!value.IsA<bool>())
        {
          ezLog::Error("Unsupported material flag: {0}", it.Key());
          continue;
        }

        flags |= value.Get<bool>() ? EZ_BIT(it.Key()) : 0;
      }

      data.m_Flags = flags;
    }
    return res;
  }

#pragma endregion
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_MaterialResource);
