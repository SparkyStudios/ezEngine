#pragma once

#pragma region spSamplerDescription

inline const spSamplerDescription spSamplerDescription::Point = {
  {},
  0,
  0xffffffff,
  0,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  ezColor(0, 0, 0),
  0,
  spSamplerFilter::Point,
  spSamplerFilter::Point,
  spSamplerFilter::Point,
  spDepthStencilComparison::Default,
};

inline const spSamplerDescription spSamplerDescription::Linear = {
  {},
  0,
  0xffffffff,
  0,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  ezColor(0, 0, 0),
  0,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spDepthStencilComparison::Default,
};

inline const spSamplerDescription spSamplerDescription::Anisotropic4x = {
  {},
  0,
  0xffffffff,
  0,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  spSamplerAddressMode::ClampToEdge,
  ezColor(0, 0, 0),
  4,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spSamplerFilter::Linear,
  spDepthStencilComparison::Default,
};

#pragma endregion
