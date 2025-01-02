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

implementing MaterialSystem;

/// Generates a surface for the given vertex from the given material data.
/// \param material The material used to generate the surface.
/// \param vertex The output of the vertex shader.
/// \param data The geometry's material data.
/// \return The geometry's surface for the given vertex.
public Surface GenerateSurface(in IMaterial material, in TransientStream vertex, in MaterialData data)
{
  Surface surface = Surface.from(data);

#ifdef USE_WORLDPOS
  surface.WorldPosition = vertex.WorldPosition;
#else
  surface.WorldPosition = float3(0.0f);
#endif

  float3 worldNormal = normalize(material.GetNormal(vertex, data));

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

  surface.Diffuse         = material.GetDiffuseColor(vertex, data);
  surface.Specular        = material.GetSpecularColor(vertex, data);
  surface.Emissive        = material.GetEmissiveColor(vertex, data);
  surface.LinearRoughness = material.GetLinearRoughness(vertex, data);
  surface.Metalness       = material.GetMetalness(vertex, data);
  surface.Occlusion       = material.GetOcclusion(vertex, data);
  surface.Cavity          = material.GetCavity(vertex, data);
  surface.Roughness2      = CalculateSpecularAA(surface.LinearRoughness, surface.WorldNormal);
  surface.Roughness       = fast_sqrt(surface.Roughness2);

  return surface;
}

// Specular Anti-Aliasing technique from this paper:
// http://www.jp.square-enix.com/tech/library/pdf/ImprovedGeometricSpecularAA.pdf
public float CalculateSpecularAA(in float roughness, in float3 normal, const float strength = 2.0f)
{
  // Constants for formula below
  static const float screenVariance = 0.25f;
  static const float varianceThresh = 0.18f;

  // Specular Anti-Aliasing
  float  roughness2       = roughness * roughness;
  float3 dndu             = ddx_fine(normal);
  float3 dndv             = ddy_fine(normal);
  float  variance         = screenVariance * (dot(dndu, dndu) + dot(dndv, dndv));
  float  kernelRoughness2 = min(strength * variance, varianceThresh);

  return saturate(roughness2 + kernelRoughness2);
}

public float3 ComputeDiffuseEnergy(in float3 F, in float metalness)
{
  // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
  // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
  // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.

  float3 kS = F;          // The energy of light that gets reflected - Equal to Fresnel
  float3 kD = 1.0f - kS;  // Remaining energy, light that gets refracted
  kD *= 1.0f - metalness; // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting

  return kD;
}

/// Decodes a value from a normal texture.
/// \param normal The sampled normal texture value.
/// \param intensity The normal intensity. Defaults to 1.0.
/// \return The decoded normal.
public float3 DecodeNormalTexture(in float4 normal, in float intensity = 1.0f)
{
  float2 xy = unpack(normal.xy);
  float  z  = sqrt(max(1.0f - dot(xy, xy), 0.0f));

  return float3(xy * saturate(intensity), z);
}

#ifdef USE_NORMAL
/// Converts normals from tangent space to world space.
/// \param normal The normal vector in tangent space.
/// \param vertex The transient vertex data.
/// \return The normal vector converted in world space.
public float3 TangentToWorldSpace(in float3 normal, in TransientStream vertex)
{
#  ifdef USE_TANGENT
  return normal.x * vertex.Tangent + normal.y * vertex.BiTangent + normal.z * vertex.Normal;
#  else
  return normal.z * vertex.Normal;
#  endif
}
#endif

/// Gets the correct alpha threshold following the vertex world poisition distance to the camera.
/// \param worldPosition The world position of the vertex.
/// \param cameraPosition The world position of the camera.
/// \return A suitable alpha threshold.
public float GetAlphaThreshold(in float3 worldPosition, in float3 cameraPosition)
{
  // Closer objects have a lower threshold, while distant objects have a higher threshold
  // This prevents things like foliage from resolving to little or no pixels over a certain distance
  float distance  = length(worldPosition - cameraPosition);
  float threshold = o_DefaultAlphaThreshold - (distance * 0.001f) * (1.0f - o_DefaultAlphaThreshold);
  threshold       = saturate(threshold);

  return threshold;
}
