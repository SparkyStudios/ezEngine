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

#include <RPI/Materials/MaterialParser.h>
#include <RPI/Resources/Loaders/RootMaterialResourceLoader.h>
#include <RPI/Resources/RootMaterialResource.h>
#include <RPI/Shaders/ShaderManager.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>

using namespace RPI;

static spRootMaterialResourceLoader s_RootMaterialResourceLoader;

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RPI, RootMaterialResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezResourceManager::SetResourceTypeLoader<spRootMaterialResource>(&s_RootMaterialResourceLoader);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezResourceManager::SetResourceTypeLoader<spRootMaterialResource>(nullptr);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

spRootMaterialResourceLoader::spRootMaterialResourceLoader()
  : ezResourceTypeLoader()
{
}

ezResourceLoadData spRootMaterialResourceLoader::OpenDataStream(const ezResource* pResource)
{
  LoadedData* pData = EZ_DEFAULT_NEW(LoadedData);

  ezResourceLoadData res;

  ezFileReader file;
  if (file.Open(pResource->GetResourceID()).Failed())
    return res;

  const ezStringBuilder sAbsolutePath = file.GetFilePathAbsolute();
  res.m_sResourceDescription = file.GetFilePathRelative().GetView();

#if EZ_ENABLED(EZ_SUPPORTS_FILE_STATS)
  {
    ezFileStats stat;
    if (ezFileSystem::GetFileStats(pResource->GetResourceID(), stat).Succeeded())
    {
      res.m_LoadedFileModificationDate = stat.m_LastModificationTime;
    }
  }
#endif

  ezMemoryStreamWriter w(&pData->m_Storage);

  if (sAbsolutePath.HasExtension("slangm"))
  {
    w.WriteVersion(spRootMaterialResource::GetResourceVersion());
    w << static_cast<ezUInt8>(0); // Compression Mode (Uncompressed)

    spMaterialMetadata metadata;
    if (spMaterialParser::ParseMaterialMetadata(sAbsolutePath, metadata).Failed())
      ezLog::Error("Unable to parse metadata for material file '{0}'", sAbsolutePath);

    w << metadata;

    auto* pShaderManager = ezSingletonRegistry::GetRequiredSingletonInstance<spShaderManager>();
    Slang::ComPtr<slang::IBlob> pBlob;

    spRootMaterialCompilerSetup setup;
    setup.m_sRootMaterialPath = sAbsolutePath;
    pShaderManager->CompileRootMaterial(setup, pBlob);

    w << pBlob->getBufferSize();
    w.WriteBytes(pBlob->getBufferPointer(), pBlob->getBufferSize()).IgnoreResult();
  }
  else if (sAbsolutePath.HasExtension("spRootMaterial"))
  {
    // skip the absolute file path data that the standard file reader writes into the stream
    {
      ezStringBuilder sAbsFilePath;
      file >> sAbsFilePath;
    }

    ezAssetFileHeader assetHash;
    assetHash.Read(file).IgnoreResult();

    ezTypeVersion v = 0;
    file.ReadWordValue(&v).IgnoreResult();

    ezUInt8 uiCompressionMode = 0;
    file >> uiCompressionMode;

    ezDynamicArray<ezUInt8> bytes;
    bytes.SetCountUninitialized(static_cast<ezUInt32>(file.GetFileSize()));

    const ezUInt64 uiReadBytes = file.ReadBytes(bytes.GetData(), file.GetFileSize());

    w.WriteVersion(v);
    w << uiCompressionMode;
    w.WriteBytes(bytes.GetData(), uiReadBytes).IgnoreResult();
  }

  res.m_pDataStream = &pData->m_Reader;
  res.m_pCustomLoaderData = pData;

  return res;
}

void spRootMaterialResourceLoader::CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData)
{
  auto* pData = static_cast<LoadedData*>(loaderData.m_pCustomLoaderData);

  if (pData == nullptr)
    return;

  pData->m_Storage.Clear();

  EZ_DEFAULT_DELETE(pData);
}

bool spRootMaterialResourceLoader::IsResourceOutdated(const ezResource* pResource) const
{
  // Don't try to reload a file that cannot be found
  ezStringBuilder sAbs;
  if (ezFileSystem::ResolvePath(pResource->GetResourceID(), &sAbs, nullptr).Failed())
    return false;

#if EZ_ENABLED(EZ_SUPPORTS_FILE_STATS)

  if (pResource->GetLoadedFileModificationTime().IsValid())
  {
    ezFileStats stat;
    if (ezFileSystem::GetFileStats(pResource->GetResourceID(), stat).Failed())
      return false;

    return !stat.m_LastModificationTime.Compare(pResource->GetLoadedFileModificationTime(), ezTimestamp::CompareMode::FileTimeEqual);
  }

#endif

  return true;
}

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Resources_Loaders_RootMaterialResourceLoader);
