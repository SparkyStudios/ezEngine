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

#include <MaterialFlags.h>

/// Describes a material for every object in the world.
struct MaterialData
{
  float4 AlbedoColor;
  float4 SpecularColor;
  float4 EmissiveColor;

  float2 UVTiling;
  float2 UVOffset;

  float Roughness;
  float Metalness;
  float NormalIntensity;
  float Height;

  float        WorldSpaceHeight;
  float        IOR;
  float        SubsurfaceScattering;
  private uint Flags;

  float4 SheenTint_Sheen;

  float Anisotropic;
  float AnisotropicRotation;
  float Clearcoat;
  float ClearcoatRoughness;

  property DiffuseColor : float3
  {
    get {
      return lerp(AlbedoColor.rgb, float3(0.0f), Metalness);
    }
  }

  property Sheen : float
  {
    get { return SheenTint_Sheen.w; }
  }

  property SheenTint : float3
  {
    get {
      return SheenTint_Sheen.xyz;
    }
  }

  property HasTextureAlbedo : bool
  {
    get { return CheckFlag(k_HasTextureAlbedoMask); }
  }

  property HasTextureNormal : bool
  {
    get { return CheckFlag(k_HasTextureNormalMask); }
  }

  property HasTextureMetalness : bool
  {
    get { return CheckFlag(k_HasTextureMetalnessMask); }
  }

  property HasTextureRoughness : bool
  {
    get { return CheckFlag(k_HasTextureRoughnessMask); }
  }

  property HasTextureCavity : bool
  {
    get { return CheckFlag(k_HasTextureCavityMask); }
  }

  property HasTextureOcclusion : bool
  {
    get { return CheckFlag(k_HasTextureOcclusionMask); }
  }

  property HasTextureORMC : bool
  {
    get { return CheckFlag(k_HasTextureORMCMask); }
  }

  property HasTextureSpecular : bool
  {
    get { return CheckFlag(k_HasTextureSpecularMask); }
  }

  property HasTextureEmissive : bool
  {
    get { return CheckFlag(k_HasTextureEmissiveMask); }
  }

  property HasTextureAlpha : bool
  {
    get { return CheckFlag(k_HasTextureAlphaMask); }
  }

  /// Checks wether the given feature (shift) is enabled in the given flag.
  /// \param flags The material flag.
  /// \param shift The feature shift in the flag.
  /// \return \c true if the feature is enabled, \c false otherwise.
  bool CheckFlag(uint shift) { return (Flags & uint(1U << shift)) == 1U; }
}
