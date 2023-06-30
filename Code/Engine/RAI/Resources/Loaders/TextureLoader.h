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

#pragma once

#include <RAI/RAIDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>

#include <RAI/Texture.h>

namespace RAI
{
  class SP_RAI_DLL spTextureResourceLoader : public ezResourceTypeLoader
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

      spImage* m_pImage{nullptr};
      spSampler* m_pSampler{nullptr};

      bool m_bColorData{false};
    };

    ezResourceLoadData OpenDataStream(const ezResource* pResource) override;
    void CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData) override;
    bool IsResourceOutdated(const ezResource* pResource) const override;

  private:
    static ezResult LoadTextureFile(ezStreamReader& inout_stream, LoadedData& ref_data);
    static void WriteTextureLoadStream(ezStreamWriter& inout_stream, const LoadedData& data);
  };
} // namespace RAI
