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

#include <Core.slang>

#define USE_WORLDPOS
#define USE_VELOCITY
#define USE_NORMAL
#define USE_TANGENT
#define USE_BITANGENT
#define USE_TEXCOORD0

#if SP_ENABLED(SP_FEATURE_VERTEX_SKINNING)
#define USE_SKINNING
#endif

#include <Common.slang>
#include <Materials.slang>
#include <RenderingContext.slang>

[shader("vertex")]
func VSMain(input: InputStream) -> TransientStream
{
  TransientStream output;

  // Set the instance ID
  output.InstanceID = input.InstanceID;

  // Get the instance data
  PerInstanceData instance = GetInstanceData(output.InstanceID);

  // Normal/Tangent/BiTangent
#ifdef USE_NORMAL
  matrix normalMatrix = TransformToMatrix(instance.NormalTransform);
  float3 normal =  unpack(input.Normal);

  output.Normal = normalize(mul((float3x3)normalMatrix, normal));

#ifdef USE_TANGENT
  float3 tangent = unpack(input.Tangent.xyz);

#ifdef USE_BITANGENT
  float3 bitangent = unpack(input.BiTangent.xyz);
#else
  float magnitude = unpack(input.Tangent.w);
  float3 bitangent = cross(normal, tangent) * magnitude;
#endif

  output.Tangent = normalize(mul((float3x3)normalMatrix, tangent));
  output.BiTangent = normalize(mul((float3x3)normalMatrix, bitangent));
#endif
#endif

  // Position
  matrix worldMatrix = TransformToMatrix(instance.WorldTransform);
  float4 worldPosition = mul(worldMatrix, float4(input.Position, 1.0));

  output.WorldPosition = worldPosition.xyz;
  output.Position = mul(Buffer_PerView.ViewProjectionMatrix, worldPosition);

#ifdef USE_VELOCITY
  matrix preiousWorldMatrix = TransformToMatrix(instance.PreviousWorldTransform);
  float4 previousWorldPosition = mul(preiousWorldMatrix, float4(input.Position, 1.0));

  output.PreviousWorldPosition = previousWorldPosition.xyz;
  output.PreviousPosition = mul(Buffer_PerView.ViewProjectionMatrix, previousWorldPosition).xyz;
#endif

  // Texture Coordinates
#ifdef USE_TEXCOORD0
  output.TexCoord0 = input.TexCoord0;

#ifdef USE_TEXCOORD1
  output.TexCoord1 = input.TexCoord1;
#endif
#endif

  return output;
}

[shader("pixel")]
func PSMain(input: TransientStream) -> float4
{
  OutputStream output;

  output.Albedo = float4(input.BiTangent.xyz, 1.0f);

  return output.Albedo;
}
