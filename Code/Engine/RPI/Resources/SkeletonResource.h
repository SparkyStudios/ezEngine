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

#include <RPI/RPIDLL.h>

#include <RPI/Assets/Skeleton.h>

#include <Core/ResourceManager/Resource.h>

namespace RPI
{
  typedef ezTypedResourceHandle<class spSkeletonResource> spSkeletonResourceHandle;

  class SP_RPI_DLL spSkeletonResourceDescriptor
  {
    friend class spSkeletonResource;

  public:
    spSkeletonResourceDescriptor();

    void Clear();

    [[nodiscard]] EZ_ALWAYS_INLINE const spSkeleton& GetSkeleton() const { return m_Skeleton; }

    EZ_ALWAYS_INLINE void SetSkeleton(const spSkeleton& skeleton) { m_Skeleton = skeleton; }

    ezResult Save(ezStreamWriter& inout_stream);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFileName);

  private:
    spSkeleton m_Skeleton;
  };

  class SP_RPI_DLL spSkeletonResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spSkeletonResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spSkeletonResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spSkeletonResource, spSkeletonResourceDescriptor);

  public:
    spSkeletonResource();

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spSkeletonResourceDescriptor m_Descriptor;
  };
} // namespace RPI
