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

  // Normal
  matrix normalMatrix = TransformToMatrix(instance.NormalTransform);
  output.Normal = normalize(mul((float3x3)normalMatrix, input.Normal));

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

  // TODO
  output.Tangent = input.Tangent;
  output.BiTangent = input.BiTangent;
  output.TexCoord0 = input.TexCoord0;

  return output;
}

[shader("pixel")]
float4 PSMain(TransientStream input)
  : SV_Target
{
  if (Buffer_PerFrame.HDREnabled)
    return float4(input.Normal.xyz, 1.0f);

  return float4(normalize(input.Normal.xyz), 1.0f);
}
