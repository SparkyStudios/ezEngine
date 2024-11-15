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

#include <RAI/RAIPCH.h>

#include <RAI/Resources/Loaders/ShaderLoader.h>
#include <RAI/Resources/ShaderResource.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>

using namespace RAI;
using namespace slang;

static spShaderResourceLoader s_ShaderResourceLoader;

ezCVarFloat cvar_Streaming_ShaderLoadDelay("Streaming.ShaderLoadDelay", 0.0f, ezCVarFlags::Save, "Artificial shader loading slowdown.");

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RAI, ShaderResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezResourceManager::SetResourceTypeLoader<spShaderResource>(&s_ShaderResourceLoader);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezResourceManager::SetResourceTypeLoader<spShaderResource>(nullptr);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

spShaderResourceLoader::spShaderResourceLoader()
  : ezResourceTypeLoader()
{
  slang::createGlobalSession(m_pGlobalCompilerSession.writeRef());
}

ezResourceLoadData spShaderResourceLoader::OpenDataStream(const ezResource* pResource)
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

  if (sAbsolutePath.HasExtension("slang"))
  {
    ezDynamicArray<ezUInt8> content;
    content.SetCountUninitialized(static_cast<ezUInt32>(file.GetFileSize()));
    const ezUInt64 uiBytesRead = file.ReadBytes(content.GetData(), content.GetCount());

    w.WriteVersion(spShaderResource::GetResourceVersion());
    w << static_cast<ezUInt8>(0); // Compression Mode (Uncompressed)
    w << uiBytesRead;
    w.WriteBytes(content.GetData(), uiBytesRead).IgnoreResult();
  }
  else if (sAbsolutePath.HasExtension("spShader"))
  {
    pData->m_bFromShaderAsset = true;

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

    pData->m_ShaderBytes = EZ_DEFAULT_NEW_ARRAY(ezUInt8, static_cast<ezUInt32>(file.GetFileSize()));
    const ezUInt64 uiReadBytes = file.ReadBytes(pData->m_ShaderBytes.GetPtr(), file.GetFileSize());

    w.WriteVersion(v);
    w << uiCompressionMode;
    w.WriteBytes(pData->m_ShaderBytes.GetPtr(), uiReadBytes).IgnoreResult();
  }

  res.m_pDataStream = &pData->m_Reader;
  res.m_pCustomLoaderData = pData;

  if (cvar_Streaming_ShaderLoadDelay > 0)
  {
    ezThreadUtils::Sleep(ezTime::Seconds(cvar_Streaming_ShaderLoadDelay));
  }

  return res;
}

void spShaderResourceLoader::CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData)
{
  auto* pData = static_cast<LoadedData*>(loaderData.m_pCustomLoaderData);

  if (pData == nullptr)
    return;

  if (pData->m_bFromShaderAsset)
    EZ_DEFAULT_DELETE_ARRAY(pData->m_ShaderBytes);

  pData->m_Storage.Clear();

  EZ_DEFAULT_DELETE(pData);
}

bool spShaderResourceLoader::IsResourceOutdated(const ezResource* pResource) const
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

Slang::ComPtr<slang::ISession> spShaderResourceLoader::CreateSession()
{
  slang::SessionDesc desc;

  desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

  slang::TargetDesc targetDesc;
  targetDesc.format = SLANG_TARGET_NONE;

  desc.targets = &targetDesc;
  desc.targetCount = 1;

  ezStringBuilder sEngineShadersPathBuilder(ezFileSystem::GetSdkRootDirectory());
  sEngineShadersPathBuilder.AppendPath("Shaders", "Lib");

  ezStringBuilder sProjectShadersPathBuilder;
  if (ezFileSystem::ResolvePath(":project/Shaders/Lib", &sProjectShadersPathBuilder, nullptr).Failed())
  {
    const char* searchPaths[] = {sEngineShadersPathBuilder.GetData()};
    desc.searchPaths = searchPaths;
    desc.searchPathCount = 1;
  }
  else
  {
    const char* searchPaths[] = {sEngineShadersPathBuilder.GetData(), sProjectShadersPathBuilder.GetData()};
    desc.searchPaths = searchPaths;
    desc.searchPathCount = 2;
  }

  Slang::ComPtr<slang::ISession> session;
  const SlangResult res = m_pGlobalCompilerSession->createSession(desc, session.writeRef());

  return res == SLANG_OK ? session : nullptr;
}
