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

#include <RAI/Shader.h>

#include <Core/ResourceManager/Resource.h>

#include <slang-com-ptr.h>

namespace RAI
{
  class SP_RAI_DLL spShaderResourceDescriptor
  {
    friend class spShaderResource;

  public:
    spShaderResourceDescriptor();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spShader& GetShader() const { return m_Shader; }
    [[nodiscard]] EZ_ALWAYS_INLINE spShader& GetShader() { return m_Shader; }

    EZ_ALWAYS_INLINE void SetShader(const spShader& shader) { m_Shader = shader; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFileName);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFileName);

  private:
    spShader m_Shader;
  };

  class SP_RAI_DLL spShaderResource final : public ezResource
  {
    friend class spShaderResourceLoader;

    EZ_ADD_DYNAMIC_REFLECTION(spShaderResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spShaderResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spShaderResource, spShaderResourceDescriptor);

  public:
    static ezTypeVersion GetResourceVersion();

    spShaderResource();

    [[nodiscard]] EZ_ALWAYS_INLINE const spShaderResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spShaderResourceDescriptor m_Descriptor;
  };

  typedef ezTypedResourceHandle<spShaderResource> spShaderResourceHandle;
} // namespace RAI
