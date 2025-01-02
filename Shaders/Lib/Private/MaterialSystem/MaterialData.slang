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

/// Describes a material for every object in the world.
public struct MaterialData
{
  public float4 AlbedoColor;
  public float4 SpecularColor;
  public float4 EmissiveColor;

  public float2 UVTiling;
  public float2 UVOffset;

  public float Roughness;
  public float Metalness;
  public float NormalIntensity;
  public float Height;

  public float WorldSpaceHeight;
  public float IOR;
  public float SubsurfaceScattering;
  private uint Flags;

  public float4 SheenTint_Sheen;

  public float Anisotropic;
  public float AnisotropicRotation;
  public float Clearcoat;
  public float ClearcoatRoughness;

  public property DiffuseColor : float3
  {
    get {
      return lerp(AlbedoColor.rgb, float3(0.0f), Metalness);
    }
  }

  public property Sheen : float
  {
    get { return SheenTint_Sheen.w; }
  }

  public property SheenTint : float3
  {
    get {
      return SheenTint_Sheen.xyz;
    }
  }

  public property HasTextureAlbedo : bool
  {
    get { return CheckFlag(k_HasTextureAlbedoMask); }
  }

  public property HasTextureNormal : bool
  {
    get { return CheckFlag(k_HasTextureNormalMask); }
  }

  public property HasTextureMetalness : bool
  {
    get { return CheckFlag(k_HasTextureMetalnessMask); }
  }

  public property HasTextureRoughness : bool
  {
    get { return CheckFlag(k_HasTextureRoughnessMask); }
  }

  public property HasTextureCavity : bool
  {
    get { return CheckFlag(k_HasTextureCavityMask); }
  }

  public property HasTextureOcclusion : bool
  {
    get { return CheckFlag(k_HasTextureOcclusionMask); }
  }

  public property HasTextureORMC : bool
  {
    get { return CheckFlag(k_HasTextureORMCMask); }
  }

  public property HasTextureSpecular : bool
  {
    get { return CheckFlag(k_HasTextureSpecularMask); }
  }

  public property HasTextureEmissive : bool
  {
    get { return CheckFlag(k_HasTextureEmissiveMask); }
  }

  public property HasTextureAlpha : bool
  {
    get { return CheckFlag(k_HasTextureAlphaMask); }
  }

  /// Checks wether the given feature (shift) is enabled in the given flag.
  /// \param flags The material flag.
  /// \param shift The feature shift in the flag.
  /// \return \c true if the feature is enabled, \c false otherwise.
  public bool CheckFlag(uint shift) { return (Flags & uint(1U << shift)) == 1U; }
}
