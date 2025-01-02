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

/// Describes a geometry surface.
///
/// This structure stores all the required data used to describe a
/// surface for each fragment.
public struct Surface
{
  /// The diffuse color.
  public float3 Diffuse;

  /// The specular color.
  public float3 Specular;

  /// The emissive color.
  public float3 Emissive;

  /// The roughness value as specified by the
  /// roughness texture.
  public float LinearRoughness;

  /// The metalness value.
  public float Metalness;

  /// The ambient occlusion value.
  public float Occlusion;

  /// Value in the range [0..1] applied on the accumulated
  /// light values to simulate cavities on surfaces.
  public float Cavity;

  /// The clearcoat value.
  /// \note A value of \c 0 disables clearcoat.
  public float Clearcoat;

  /// The roughness value for clearcoat shading.
  /// \note Only applicable if \a Clearcoat > \c 0.
  public float ClearcoatRoughness;

  /// Normal vector used for the clearcoat shading.
  /// \note Only applicable in \a Clearcoat > \c 0.
  public float3 ClearcoatNormal;

  /// The anisotropic value.
  /// \note A value of \c 0 disables anisotropy.
  public float Anisotropic;

  /// Noise value used in the anisotropic shading.
  /// \note Only applicable if \a Anisotropic > \c 0.
  public float AnisotropicRotation;

  /// The specular sheen factor.
  /// \note A value of \c 0 disables specular sheen.
  public float Sheen;

  /// The specular sheen tint factor.
  /// \note Only applcicable if \a Sheen > \c 0.
  public float SheenTintFactor;

  public float Roughness;
  public float Roughness2;
  public float DiffuseOcclusion;
  public float SpecularOcclusion;
  public float SpecularF0;
  public float Opacity;

  /// The position of this surface fragment in world space..
  public float3 WorldPosition;

  /// The normal vector of this surface fragment in world space.
  public float3 WorldNormal;

  /// The normal vector of this surface fragment in geometry space.
  public float3 VertexNormal;

  /// The total accumulated specular energy.
  public float3 kS;

  /// The total accumulated diffuse energy.
  public float3 kD;

  public static Surface from(MaterialData material)
  {
    Surface result;

    result.kS = float3(1.0f);
    result.kD = float3(1.0f);

    result.Diffuse         = material.DiffuseColor;
    result.LinearRoughness = material.Roughness;
    result.Metalness       = material.Metalness;

    result.Roughness  = pow2(result.LinearRoughness);
    result.Roughness2 = pow2(result.Roughness);

    result.Clearcoat          = material.Clearcoat;
    result.ClearcoatRoughness = material.ClearcoatRoughness;

    result.Anisotropic         = material.Anisotropic;
    result.AnisotropicRotation = material.AnisotropicRotation;

    result.Sheen = material.Sheen;

    result.Opacity = material.AlbedoColor.a;

    return result;
  }
}
