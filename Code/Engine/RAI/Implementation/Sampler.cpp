#include <RAI/RAIPCH.h>

#include <RAI/Sampler.h>

using namespace RAI;

RAI::spSampler RAI::spSampler::CreatePoint()
{
  return spSampler(RHI::spSamplerDescription::Point);
}

RAI::spSampler RAI::spSampler::CreateLinear()
{
  return spSampler(RHI::spSamplerDescription::Linear);
}

RAI::spSampler RAI::spSampler::CreateAnisotropic4x()
{
  return spSampler(RHI::spSamplerDescription::Anisotropic4x);
}
