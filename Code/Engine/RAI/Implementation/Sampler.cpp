#include <RAI/RAIPCH.h>

#include <RAI/Sampler.h>

namespace RAI
{
  spSampler spSampler::CreatePoint()
  {
    return spSampler(spSamplerDescription::Point);
  }

  spSampler spSampler::CreateLinear()
  {
    return spSampler(spSamplerDescription::Linear);
  }

  spSampler spSampler::CreateAnisotropic4x()
  {
    return spSampler(spSamplerDescription::Anisotropic4x);
  }
} // namespace RAI
