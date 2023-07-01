#include <RAI/RAIPCH.h>

#include <RAI/Sampler.h>

#include <RHI/Device.h>

namespace RAI
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

  void spSampler::CreateRHISampler()
  {
    if (m_RHISampler != nullptr)
      return;

    auto* pDevice = ezSingletonRegistry::GetSingletonInstance<RHI::spDevice>();
    m_RHISampler = pDevice->GetResourceFactory()->CreateSampler(m_Description);
  }
} // namespace RAI

EZ_STATICLINK_FILE(RAI, RAI_Implementation_Sampler);
