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

    explicit spSampler(RHI::spSamplerDescription description)
      : m_Description(std::move(description))
    {
    }

    EZ_NODISCARD EZ_ALWAYS_INLINE const RHI::spSamplerDescription& GetDescription() const { return m_Description; }

    EZ_ALWAYS_INLINE void SetDescription(RHI::spSamplerDescription value) { m_Description = std::move(value); }

  private:
    RHI::spSamplerDescription m_Description;
  };
} // namespace RAI

inline ezStreamWriter& operator<<(ezStreamWriter& inout_stream, const RAI::spSampler& sampler)
{
  const auto& desc = sampler.GetDescription();

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
  RHI::spSamplerDescription desc;

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

  ref_sampler.SetDescription(std::move(desc));

  return inout_stream;
}
