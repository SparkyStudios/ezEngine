// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <Common/RenderingContext.slang>

static constexpr uint kHasTextureAlbedoFlagShift = 0;
static constexpr uint kHasTextureNormalFlagShift = 1;
static constexpr uint kHasTextureMetalnessFlagShift = 2;
static constexpr uint kHasTextureRoughnessFlagShift = 3;
static constexpr uint kHasTextureOcclusionFlagShift = 4;
static constexpr uint kHasTextureCavityFlagShift = 5;
static constexpr uint kHasTextureORMCFlagShift = 6;

/// Checks wether the given feature (shift) is enabled in the given flag.
/// \param flags The material flag.
/// \param shift The feature shift in the flag.
/// \return \c true if the feature is enabled, \c false otherwise.
func CheckMaterialFlag(in flags: uint, in shift: uint) -> bool
{
  return (flags & uint(1U << shift)) == 1U;
}

/// A material interface describing all the necessary methods used to generate
/// surface data for a geometry.
///
/// Its the responsibility of the material implementation to ensure valid data are returned.
interface IMaterial
{
  /// Gets the normal vector.
  /// \param data The geometry's material data.
  /// \return The surface's normal vector.
  func GetNormal(in data: MaterialData) -> float3;

  /// Gets the albedo color.
  /// \param data The geometry's material data.
  /// \return The surface's albedo color.
  func GetAlbedoColor(in data: MaterialData) -> float3;

  /// Gets the specular color.
  /// \param data The geometry's material data.
  /// \return The surface's specular color.
  func GetSpecularColor(in data: MaterialData) -> float3;

  /// Gets the emissive color.
  /// \param data The geometry's material data.
  /// \return The surface's emissive color.
  func GetEmissiveColor(in data: MaterialData) -> float3;

  /// Gets the linear roughness.
  /// \param data The geometry's material data.
  /// \return The surface's linear roughness.
  func GetLinearRoughness(in data: MaterialData) -> float;

  /// Gets the opacity.
  /// \param data The geometry's material data.
  /// \return The surface's opacity.
  func GetOpacity(in data: MaterialData) -> float;

  /// Gets the ambient occlusion.
  /// \param data The geometry's material data.
  /// \return The surface's ambient occlusion.
  func GetOcclusion(in data: MaterialData) -> float;

  /// Gets the cavity factor.
  /// \param data The geometry's material data.
  /// \return The surface's cavity.
  func GetCavity(in data: MaterialData) -> float;

  /// Get the metalness.
  /// \param data The geometry's material data.
  /// \return The surface's metalness.
  func GetMetalness(in data: MaterialData) -> float;
}

/// Describes a material for every object in the world.
struct MaterialData
{
  float4 Color;

  float2 UVTiling;
  float2 UVOffset;

  float Roughness;
  float Metalness;
  float NormalIntensity;
  float Height;

  float WorldSpaceHeight;
  float IOR;
  float SubsurfaceScattering;
  private uint Flags;

  float Sheen;
  float3 SheenTint;

  float Anisotropic;
  float AnisotropicRotation;
  float Clearcoat;
  float ClearcoatRoughness;

  property HasTextureAlbedo: bool
  {
    get { return CheckMaterialFlag(Flags, kHasTextureAlbedoFlagShift); }
  }

  property HasTextureNormal: bool
  {
    get { return CheckMaterialFlag(Flags,kHasTextureNormalFlagShift); }
  }

  // TODO: Other flags
}

[[vk_binding(4, 0)]]
readonly StructuredBuffer<MaterialData> Buffer_Materials : register(b4, space0);

/// Returns the material instance for the current rendering object.
func GetMaterial() -> MaterialData
{
  return Buffer_Materials[GetMaterialIndex()];
}

// Specular Anti-Aliasing technique from this paper:
// http://www.jp.square-enix.com/tech/library/pdf/ImprovedGeometricSpecularAA.pdf
func CalculateSpecularAA(roughness: float, normal: float3, const float strength = 2.0f) -> float
{
    // Constants for formula below
    static const float screenVariance = 0.25f;
    static const float varianceThresh = 0.18f;

    // Specular Anti-Aliasing
    float roughness2            = roughness * roughness;
    float3 dndu                 = ddx_fine(normal);
    float3 dndv                 = ddy_fine(normal);
    float variance              = screenVariance * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2      = min(strength * variance, varianceThresh);

    return saturate(roughness2 + kernelRoughness2);
}

func ComputeDiffuseEnergy(F: float3, metalness: float) -> float3
{
  // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
  // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
  // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.

  float3 kS = F;            // The energy of light that gets reflected - Equal to Fresnel
  float3 kD = 1.0f - kS;    // Remaining energy, light that gets refracted
  kD *= 1.0f - metalness;   // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting

  return kD;
}
