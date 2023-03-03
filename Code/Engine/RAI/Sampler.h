#pragma once

#include <RAI/RAIDLL.h>

#include <RHI/Sampler.h>

namespace RAI
{
  /// \brief A texture sampler asset.
  class SP_RAI_DLL spSampler
  {
    friend class spSamplerResource;
    friend class spSamplerResourceDescriptor;

  public:
    static spSampler CreatePoint();

    static spSampler CreateLinear();

    static spSampler CreateAnisotropic4x();

    spSampler() = default;

    explicit spSampler(spSamplerDescription description)
      : m_SamplerDescription(std::move(description))
    {
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE const spSamplerDescription& GetSamplerDescription() const { return m_SamplerDescription; }

    EZ_ALWAYS_INLINE void SetSamplerDescription(spSamplerDescription value) { m_SamplerDescription = std::move(value); }

  private:
    spSamplerDescription m_SamplerDescription;
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spSampler& sampler)
{
  const auto& desc = sampler.GetSamplerDescription();

  inout_stream << desc.m_uiMinLod;
  inout_stream << desc.m_uiMaxLod;
  inout_stream << desc.m_fLodBias;
  inout_stream << desc.m_eAddressModeS;
  inout_stream << desc.m_eAddressModeT;
  inout_stream << desc.m_eAddressModeR;
  inout_stream << desc.m_BorderColor;
  inout_stream << desc.m_uiMaxAnisotropy;
  inout_stream << desc.m_eMinFilter;
  inout_stream << desc.m_eMagFilter;
  inout_stream << desc.m_eMipFilter;
  inout_stream << desc.m_eSamplerComparison;

  return inout_stream;
}

inline ezStreamReader& operator>>(ezStreamReader& inout_stream, RAI::spSampler& ref_sampler)
{
  spSamplerDescription desc;

  inout_stream >> desc.m_uiMinLod;
  inout_stream >> desc.m_uiMaxLod;
  inout_stream >> desc.m_fLodBias;
  inout_stream >> desc.m_eAddressModeS;
  inout_stream >> desc.m_eAddressModeT;
  inout_stream >> desc.m_eAddressModeR;
  inout_stream >> desc.m_BorderColor;
  inout_stream >> desc.m_uiMaxAnisotropy;
  inout_stream >> desc.m_eMinFilter;
  inout_stream >> desc.m_eMagFilter;
  inout_stream >> desc.m_eMipFilter;
  inout_stream >> desc.m_eSamplerComparison;

  ref_sampler.SetSamplerDescription(desc);

  return inout_stream;
}
