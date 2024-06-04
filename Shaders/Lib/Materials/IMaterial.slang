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

#include <Common/AngularInfo.slang>
#include <Common/Surface.slang>
#include <Materials/Streams.slang>

extern static bool o_FlipWinding = false;
extern static bool o_TwoSided = false;

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