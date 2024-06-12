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

float3 F_Schlick(const float3 f0, float f90, float VoH)
{
  // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
  return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float3 F_Schlick(const float3 f0, float VoH)
{
  float f = pow(1.0 - VoH, 5.0);
  return f + f0 * (1.0 - f);
}

float3 F_Schlick_Roughness(const float3 f0, float VoH, float roughness)
{
  float3 a = 1.0 - roughness;
  return f0 + (max(a, f0) - f0) * pow(max(1.0 - VoH, 0.0), 5.0);
}
