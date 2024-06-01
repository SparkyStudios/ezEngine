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

#include <RenderingContext.slang>

/// \brief Describes a material for every object in the world.
struct Material
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
  uint Flags;

  float Sheen;
  float3 SheenTint;

  float Anisotropic;
  float AnisotropicRotation;
  float Clearcoat;
  float ClearcoatRoughness;
}

RWStructuredBuffer<Material> Buffer_Materials;

/// \brief Returns the material instance for the current rendering object.
Material GetMaterial()
{
  return Buffer_Materials[GetMaterialIndex()];
}

