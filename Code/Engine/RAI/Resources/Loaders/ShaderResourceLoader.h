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

#include <RAI/Core.h>
#include <RAI/Shader.h>

struct mpack_node_t;

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

      ezStringView m_sName;
      ezHybridArray<RHI::spInputElementDescription, 8> m_InputElements;
      ezDynamicArray<ezUInt8> m_Buffer;
      ezMap<RHI::spShaderStage::Enum, ezStringView> m_EntryPoints;
      ezDynamicArray<spPermutationVar> m_Permutations;
      RHI::spRenderingState m_RenderingState;
    };

    ezResourceLoadData OpenDataStream(const ezResource* pResource) override;
    void CloseDataStream(const ezResource* pResource, const ezResourceLoadData& loaderData) override;
    bool IsResourceOutdated(const ezResource* pResource) const override;

  private:
    static ezResult DeserializeShader(mpack_node_t root, spShaderResourceLoader::LoadedData& ref_data);
    static ezResult DeserializeInputElement(mpack_node_t root, RHI::spInputElementDescription& ref_element);
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RHI::spInputElementDescription& description)
{
  inout_stream << description.m_sName;
  inout_stream << description.m_eSemantic;
  inout_stream << description.m_eFormat;
  inout_stream << description.m_uiOffset;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RHI::spInputElementDescription& ref_description)
{
  inout_stream >> ref_description.m_sName;
  inout_stream >> ref_description.m_eSemantic;
  inout_stream >> ref_description.m_eFormat;
  inout_stream >> ref_description.m_uiOffset;

  return inout_stream;
}
