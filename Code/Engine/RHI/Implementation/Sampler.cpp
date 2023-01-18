#include "RHI/Core.h"
#include <RHI/RHIPCH.h>

#include <RHI/Sampler.h>

#pragma region spSamplerDescription

const spSamplerDescription spSamplerDescription::Point = {
  {},
  0,
  0xffffffff,
  0,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  ezColor::Black,
  0,
  spSamplerFilter::Point,
  spSamplerFilter::Point,
  spSamplerFilter::Point,
  spDepthStencilComparison::Default,
};

const spSamplerDescription spSamplerDescription::Linear = {
  {},
  0,
  0xffffffff,
  0,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  ezColor::Black,
  0,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spDepthStencilComparison::Default,
};

const spSamplerDescription spSamplerDescription::Anisotropic4x = {
  {},
  0,
  0xffffffff,
  0,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  ezColor::Black,
  4,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spDepthStencilComparison::Default,
};

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Sampler);
