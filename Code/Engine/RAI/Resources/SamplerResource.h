#pragma once

#include <RAI/RAIDLL.h>

#include <RAI/Sampler.h>

#include <Core/ResourceManager/Resource.h>

namespace RAI
{
  typedef ezTypedResourceHandle<class spSamplerResource> spSamplerResourceHandle;

  class SP_RAI_DLL spSamplerResourceDescriptor
  {
    friend class spSamplerResource;

  public:
    spSamplerResourceDescriptor();

    void Clear();

    EZ_NODISCARD EZ_ALWAYS_INLINE const spSampler& GetSampler() const { return m_Sampler; }

    EZ_ALWAYS_INLINE void SetSampler(const spSampler& sampler) { m_Sampler = sampler; }

    ezResult Save(ezStreamWriter& inout_stream);
    ezResult Save(ezStringView sFile);

    ezResult Load(ezStreamReader& inout_stream);
    ezResult Load(ezStringView sFile);

  private:
    spSampler m_Sampler;
  };

  class SP_RAI_DLL spSamplerResource final : public ezResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spSamplerResource, ezResource);
    EZ_RESOURCE_DECLARE_COMMON_CODE(spSamplerResource);
    EZ_RESOURCE_DECLARE_CREATEABLE(spSamplerResource, spSamplerResourceDescriptor);

  public:
    spSamplerResource();

  private:
    ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
    ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
    void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

    spSamplerResourceDescriptor m_Descriptor;
  };
}
