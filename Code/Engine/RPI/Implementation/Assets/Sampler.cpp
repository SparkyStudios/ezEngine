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

#include <RPI/RPIPCH.h>

#include <RPI/Assets/Sampler.h>

#include <RHI/Device.h>

namespace RPI
{
  spSampler spSampler::CreatePoint()
  {
    return spSampler(RHI::spSamplerDescription::Point);
  }

  spSampler spSampler::CreateLinear()
  {
    return spSampler(RHI::spSamplerDescription::Linear);
  }

  spSampler spSampler::CreateAnisotropic4x()
  {
    return spSampler(RHI::spSamplerDescription::Anisotropic4x);
  }

  spSampler::~spSampler() noexcept
  {
    m_RHISampler.Clear();
  }

  void spSampler::CreateRHISampler()
  {
    if (m_RHISampler != nullptr)
      return;

    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();
    m_RHISampler = pDevice->GetResourceFactory()->CreateSampler(m_Description);
  }
} // namespace RPI

EZ_STATICLINK_FILE(RPI, RPI_Implementation_Assets_Sampler);
