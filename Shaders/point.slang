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

#define USE_WORLDPOS
#define USE_NORMAL
#define USE_TANGENT
#define USE_BITANGENT
#define USE_TEXCOORD0

#include <Common.slang>
#include <Materials.slang>
#include <RenderingContext.slang>

[shader("vertex")]
TransientStream VSMain(InputStream input)
{
  TransientStream output;

  float4x4 objectToWorld = Buffer_PerPass.Transform;

  float4 wp = mul(objectToWorld, float4(input.Position, 1.0));
  output.WorldPosition = wp.xyz;
  float4 oc = mul(Buffer_PerView.ViewMatrix, wp);
  output.Position = mul(Buffer_PerView.ProjectionMatrix, oc);
  output.Normal = input.Normal;
  output.Tangent = input.Tangent;
  output.BiTangent = input.BiTangent;
  output.TexCoord0 = input.TexCoord0;
  output.InstanceID = input.InstanceID;

  return output;
}


[shader("pixel")]
float4 PSMain(TransientStream input)
  : SV_Target
{
  return float4(input.Normal.xyz, 1.0f);
}
