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

#include <Common/Math/Utils.slang>

#include <Materials/MaterialData.slang>

/// Describes a geometry surface.
///
/// This structure stores all the required data used to describe a
/// surface for each fragment.
struct Surface
{
  /// The diffuse color.
  float3 Diffuse;

  /// The specular color.
  float3 Specular;

  /// The emissive color.
  float3 Emissive;

  /// The roughness value as specified by the
  /// roughness texture.
  float LinearRoughness;

  /// The metalness value.
  float Metalness;

  /// The ambient occlusion value.
  float Occlusion;

  /// Value in the range [0..1] applied on the accumulated
  /// light values to simulate cavities on surfaces.
  float Cavity;

  /// The clearcoat value.
  /// \note A value of \c 0 disables clearcoat.
  float Clearcoat;

  /// The roughness value for clearcoat shading.
  /// \note Only applicable if \a Clearcoat > \c 0.
  float ClearcoatRoughness;

  /// Normal vector used for the clearcoat shading.
  /// \note Only applicable in \a Clearcoat > \c 0.
  float3 ClearcoatNormal;

  /// The anisotropic value.
  /// \note A value of \c 0 disables anisotropy.
  float Anisotropic;

  /// Noise value used in the anisotropic shading.
  /// \note Only applicable if \a Anisotropic > \c 0.
  float AnisotropicRotation;

  /// The specular sheen factor.
  /// \note A value of \c 0 disables specular sheen.
  float Sheen;

  /// The specular sheen tint factor.
  /// \note Only applcicable if \a Sheen > \c 0.
  float SheenTintFactor;

  float Roughness;
  float Roughness2;
  float DiffuseOcclusion;
  float SpecularOcclusion;
  float SpecularF0;
  float Opacity;

  /// The position of this surface fragment in world space..
  float3 WorldPosition;

  /// The normal vector of this surface fragment in world space.
  float3 WorldNormal;

  /// The normal vector of this surface fragment in geometry space.
  float3 VertexNormal;

  /// The total accumulated speculr energy.
  float3 kS;

  /// The total accumulated diffuse energy.
  float3 kD;

  __init()
  {
    kS = float3(1.0f);
    kD = float3(1.0f);
  }

  __init(MaterialData material)
  {
    kS = float3(1.0f);
    kD = float3(1.0f);

    Diffuse         = material.DiffuseColor;
    LinearRoughness = material.Roughness;
    Metalness       = material.Metalness;

    Roughness  = pow2(LinearRoughness);
    Roughness2 = pow2(Roughness);

    Clearcoat          = material.Clearcoat;
    ClearcoatRoughness = material.ClearcoatRoughness;

    Anisotropic         = material.Anisotropic;
    AnisotropicRotation = material.AnisotropicRotation;

    Sheen = material.Sheen;

    Opacity = material.AlbedoColor.a;
  }
}
