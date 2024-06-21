#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Skeleton.h>

#include <Core/ResourceManager/Resource.h>

namespace RAI
{
  typedef ezTypedResourceHandle<class spSkeletonResource> spSkeletonResourceHandle;

  class SP_RAI_DLL spSkeletonResourceDescriptor
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

  class SP_RAI_DLL spSkeletonResource final : public ezResource
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
} // namespace RAI
