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

/// Vertex buffer input.
struct InputStream
{
  float3 Position : POSITION;

#ifdef USE_NORMAL
  float3 Normal : NORMAL;
#endif

#ifdef USE_TANGENT
  float4 Tangent : TANGENT;

#ifdef USE_BITANGENT
  float4 BiTangent : BITANGENT;
#endif
#endif

#ifdef USE_TEXCOORD0
  float2 TexCoord0 : TEXCOORD0;

#ifdef USE_TEXCOORD1
  float2 TexCoord1 : TEXCOORD1;
#endif
#endif

#ifdef USE_COLOR0
  float4 Color0 : COLOR0;

#ifdef USE_COLOR1
  float4 Color1 : COLOR1;
#endif
#endif

#ifdef USE_SKINNING
  float4 BoneWeigths : BONEWEIGHTS0;
  uint4  BoneIndices : BONEINDICES0;
#endif
}

/// Transient data between shader stages.
struct TransientStream
{
  float4 Position : SV_Position;

#ifdef USE_VELOCITY
  float3 PreviousPosition : PREVPOS;
#endif

#ifdef USE_WORLDPOS
  float3 WorldPosition : WORLDPOS;

#ifdef USE_VELOCITY
  float3 PreviousWorldPosition : PREVWORLDPOS;
#endif
#endif

#ifdef USE_NORMAL
  float3 Normal : NORMAL;
#endif

#ifdef USE_TANGENT
  float3 Tangent : TANGENT;
  float3 BiTangent : BITANGENT;
#endif

#ifdef USE_TEXCOORD0
  float2 TexCoord0 : TEXCOORD0;

#ifdef USE_TEXCOORD1
  float2 TexCoord1 : TEXCOORD1;
#endif
#endif

#ifdef USE_COLOR0
  float4 Color0 : COLOR0;

#ifdef USE_COLOR1
  float4 Color1 : COLOR1;
#endif
#endif

  uint InstanceID  : InstanceID;
  bool IsFrontFace : IsFrontFace;
}

/// G-Buffer output.
struct OutputStream
{
  float4 Albedo   : SV_Target0;
  float4 Normal   : SV_Target1;
  float4 Material : SV_Target2;
  float2 Velocity : SV_Target3;
}
