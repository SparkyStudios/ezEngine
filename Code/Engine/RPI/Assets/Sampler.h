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

#include <RPI/RPIDLL.h>

#include <RHI/Sampler.h>

namespace RPI
{
  /// \brief A texture sampler asset.
  class SP_RPI_DLL spSampler
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

    ~spSampler() noexcept;

    [[nodiscard]] EZ_ALWAYS_INLINE const RHI::spSamplerDescription& GetDescription() const { return m_Description; }

    EZ_ALWAYS_INLINE void SetDescription(RHI::spSamplerDescription value) { m_Description = std::move(value); }

    void CreateRHISampler();

    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<RHI::spSampler> GetRHISampler() const { return m_RHISampler; }

  private:
    RHI::spSamplerDescription m_Description;
    ezSharedPtr<RHI::spSampler> m_RHISampler{nullptr};
  };
} // namespace RPI

#include <RPI/Implementation/Assets/Sampler_inl.h>
