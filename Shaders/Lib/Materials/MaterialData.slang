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
    get { return CheckMaterialFlag(Flags, kHasTextureNormalFlagShift); }
  }

  // TODO: Other flags
}

[[vk_binding(4, 0)]]
readonly StructuredBuffer<MaterialData> Buffer_Materials : register(b4, space0);
