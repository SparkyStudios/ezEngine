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

#pragma once

#include <RAI/RAIDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>

#include <RAI/Shader.h>

#include <slang-com-ptr.h>

namespace RAI
{
  class SP_RAI_DLL spShaderResourceLoader : public ezResourceTypeLoader
  {
  public:
    struct LoadedData
    {
      LoadedData()
        : m_Reader(&m_Storage)
      {
      }

      ezContiguousMemoryStreamStorage m_Storage;
      ezMemoryStreamReader m_Reader;

      bool m_bFromShaderAsset{false};
      ezByteArrayPtr m_ShaderBytes;
    };

    spShaderResourceLoader();

    ezResourceLoadData OpenDataStream(const ezResource* pResource) override;
    void CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData) override;
    bool IsResourceOutdated(const ezResource* pResource) const override;

  private:
    Slang::ComPtr<slang::ISession> CreateSession();

    Slang::ComPtr<slang::IGlobalSession> m_pGlobalCompilerSession{nullptr};
  };
} // namespace RAI
