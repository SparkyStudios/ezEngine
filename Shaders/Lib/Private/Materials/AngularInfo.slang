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

#include <Common/Light.slang>
#include <Common/RenderingContext.slang>

#include <Materials/Surface.slang>

/// Stores precomputed values about common vectors between
/// the light source and the geometry surface.
struct AngularInfo
{
  /// Outward direction of the geometry surface.
  float3 N;

  /// Direction of the vector from the light source to the geometry surface.
  float3 L;

  /// Direction of the vector from the camera to the geometry surface.
  float3 V;

  /// Direction of the vector between L and V
  float3 H;

  float LoH;
  float VoH;
  float NoV;
  float NoH;
  float NoL;

  __init(Light light, Surface surface, float3 cameraPosition)
  {
    N = normalize(surface.WorldNormal);
    L = normalize(light.Position - surface.WorldPosition);
    V = normalize(cameraPosition - surface.WorldPosition);
    H = normalize(L + V);

    NoL = saturate(dot(N, L));
    NoV = saturate(dot(N, V));
    NoH = saturate(dot(N, H));
    LoH = saturate(dot(L, H));
    VoH = saturate(dot(V, H));
  }
}
