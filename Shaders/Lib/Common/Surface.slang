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

struct Surface
{
  float3 albedo;
  float3 specular;
  float3 emissive;
  float roughnessLinear;
  float roughness;
  float roughness2;
  float metalness;
  float diffuseOcclusion;
  float speculatOcclusion;
  float specularF0;
  float cavity;
  float opacity;

  float clearcoat;
  float clearcoatRoughness;
  float3 clearcoatNormal;

  float anisotropic;
  float anisotropicRotation;

  float sheen;
  float sheenTintFactor;

  float3 worldPosition;
  float3 worldNormal;
  float3 vertexNormal;

  float3 kS;
  float3 kD;

  __init()
  {
    kS = float3(1.0f);
    kD = float3(1.0f);
  }
}
