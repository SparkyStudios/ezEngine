#include <RAI/RAIPCH.h>

#include <RAI/Sampler.h>

using namespace RAI;

RAI::spSampler RAI::spSampler::CreatePoint()
{
  return spSampler(spSamplerDescription::Point);
}

RAI::spSampler RAI::spSampler::CreateLinear()
{
  return spSampler(spSamplerDescription::Linear);
}

RAI::spSampler RAI::spSampler::CreateAnisotropic4x()
{
  return spSampler(spSamplerDescription::Anisotropic4x);
}
