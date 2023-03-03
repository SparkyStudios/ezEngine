#include <RAI/RAIPCH.h>

#include <RAI/Resources/SamplerResource.h>

#include <Core/Assets/AssetFileHeader.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

namespace RAI
{
  // clang-format off
  EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(spSamplerResource, 1, ezRTTIDefaultAllocator<spSamplerResource>)
  EZ_END_DYNAMIC_REFLECTED_TYPE;

  EZ_RESOURCE_IMPLEMENT_COMMON_CODE(spSamplerResource);
  // clang-format on

#pragma region spSamplerResourceDescriptor

  static constexpr ezUInt16 kSamplerResourceVersion = 1;

  spSamplerResourceDescriptor::spSamplerResourceDescriptor()
  {
    Clear();
  }

  void spSamplerResourceDescriptor::Clear()
  {
    m_Sampler.SetSamplerDescription(spSamplerDescription::Linear);
  }

  ezResult spSamplerResourceDescriptor::Save(ezStreamWriter& inout_stream)
  {
    inout_stream << kSamplerResourceVersion;
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
    ezUInt16 uiVersion = 0;
    inout_stream >> uiVersion;

    if (uiVersion == 0 || uiVersion > kSamplerResourceVersion)
      return EZ_FAILURE;

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
    m_Descriptor.Clear();

    ezResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 1;
    res.m_State = ezResourceState::Unloaded;

    return res;
  }

  ezResourceLoadDesc spSamplerResource::UpdateContent(ezStreamReader* pStream)
  {
    spSamplerResourceDescriptor desc;

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

  void spSamplerResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
  {
    // TODO
    out_NewMemoryUsage.m_uiMemoryCPU = sizeof(spSamplerResource);
    out_NewMemoryUsage.m_uiMemoryGPU = 0;
  }

  EZ_RESOURCE_IMPLEMENT_CREATEABLE(spSamplerResource, spSamplerResourceDescriptor)
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
