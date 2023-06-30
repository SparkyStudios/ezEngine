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

#include <RAI/Image.h>

#include <Core/ResourceManager/Resource.h>

namespace RAI
{
  typedef ezTypedResourceHandle<class spImageResource> spImageResourceHandle;

  class SP_RAI_DLL spImageResourceDescriptor
  {
    friend class spImageResource;

  public:
    spImageResourceDescriptor();

    void Clear();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spImage& GetImage() const { return m_Image; }
    EZ_NODISCARD EZ_ALWAYS_INLINE spImage& GetImage() { return m_Image; }

    EZ_NODISCARD void SetImage(const spImage& image);

    ezResult Save(ezStreamWriter& inout_stream);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFileName);

  private:
    spImage m_Image;
  };

  class SP_RAI_DLL spImageResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spImageResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spImageResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spImageResource, spImageResourceDescriptor);

  public:
    spImageResource();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spImageResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spImageResourceDescriptor m_Descriptor;
  };
} // namespace RAI
