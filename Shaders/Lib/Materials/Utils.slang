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

#include <Materials/AngularInfo.slang>
#include <Materials/IMaterial.slang>
#include <Materials/Streams.slang>
#include <Materials/Surface.slang>

extern static bool o_FlipWinding = false;
extern static bool o_TwoSided = false;

/// Returns the material instance for the current rendering object.
/// \param materialID The ID of the material to get.
func GetMaterial(in materialID: uint) -> MaterialData
{
  return Buffer_Materials[materialID];
}

/// Generates a surface for the given vertex from the given material data.
/// \param material The material used to generate the surface.
/// \param vertex The output of the vertex shader.
/// \param data The geometry's material data.
/// \return The geometry's surface for the given vertex.
func GenerateSurface(in material: IMaterial, in vertex: TransientStream, in data: MaterialData) -> Surface
{
  Surface surface = Surface(data);

#ifdef USE_WORLDPOS
  surface.WorldPosition = vertex.WorldPosition;
#else
  surface.WorldPosition = float3(0.0f);
#endif

  float3 worldNormal = normalize(material.GetNormal(data));

  if (o_TwoSided)
  {
    if (o_FlipWinding)
      surface.WorldNormal = vertex.IsFrontFace ? -worldNormal : worldNormal;
    else
      surface.WorldNormal = vertex.IsFrontFace ? worldNormal : -worldNormal;
  }
  else
  {
    surface.WorldNormal = worldNormal;
  }

#ifdef USE_NORMAL
  surface.VertexNormal = normalize(vertex.Normal);
#else
  surface.VertexNormal = float3(0.0f, 0.0f, 1.0f);
#endif

  surface.Albedo = material.GetAlbedoColor(data);
  surface.Specular = material.GetSpecularColor(data);
  surface.Emissive = material.GetEmissiveColor(data);
  surface.LinearRoughness = material.GetLinearRoughness(data);
  surface.Metalness = material.GetMetalness(data);
  surface.Occlusion = material.GetOcclusion(data);
  surface.Cavity = material.GetCavity(data);
  surface.Roughness = pow2(surface.LinearRoughness);
  surface.Roughness2 = pow2(surface.Roughness);

  return surface;
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

func ComputeDiffuseEnergy(in F: float3, in metalness: float) -> float3
{
  // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
  // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
  // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.

  float3 kS = F;            // The energy of light that gets reflected - Equal to Fresnel
  float3 kD = 1.0f - kS;    // Remaining energy, light that gets refracted
  kD *= 1.0f - metalness;   // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting

  return kD;
}
