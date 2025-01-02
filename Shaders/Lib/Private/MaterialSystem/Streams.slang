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

implementing MaterialSystem;

/// Vertex buffer input.
public struct InputStream
{
  float3 Position : POSITION;

#ifdef USE_NORMAL
  public float3 Normal : NORMAL;
#endif

#ifdef USE_TANGENT
  public float4 Tangent : TANGENT;

#  ifdef USE_BITANGENT
  public float4 BiTangent : BITANGENT;
#  endif
#endif

#ifdef USE_TEXCOORD0
  public float2 TexCoord0 : TEXCOORD0;

#  ifdef USE_TEXCOORD1
  public float2 TexCoord1 : TEXCOORD1;
#  endif
#endif

#ifdef USE_COLOR0
  public float4 Color0 : COLOR0;

#  ifdef USE_COLOR1
  public float4 Color1 : COLOR1;
#  endif
#endif

#ifdef USE_SKINNING
  public float4 BoneWeigths : BONEWEIGHTS0;
  public uint4  BoneIndices : BONEINDICES0;
#endif
}

/// Transient data between shader stages.
public struct TransientStream
{
  public float4 Position : SV_Position;

#ifdef USE_VELOCITY
  public float4 PreviousPosition : PREVPOS;
#endif

#ifdef USE_WORLDPOS
  public float3 WorldPosition : WORLDPOS;

#  ifdef USE_VELOCITY
  public float3 PreviousWorldPosition : PREVWORLDPOS;
#  endif
#endif

#ifdef USE_NORMAL
  public float3 Normal : NORMAL;
#endif

#ifdef USE_TANGENT
  public float3 Tangent : TANGENT;
  public float3 BiTangent : BITANGENT;
#endif

#ifdef USE_TEXCOORD0
  public float2 TexCoord0 : TEXCOORD0;

#  ifdef USE_TEXCOORD1
  public float2 TexCoord1 : TEXCOORD1;
#  endif
#endif

#ifdef USE_COLOR0
  public float4 Color0 : COLOR0;

#  ifdef USE_COLOR1
  public float4 Color1 : COLOR1;
#  endif
#endif

  public uint InstanceID : InstanceID;
  public bool IsFrontFace : IsFrontFace;
}

/// G-Buffer output.
public struct OutputStream
{
  public float4 Diffuse : SV_Target0;
  public float4 Normal : SV_Target1;
  public float4 Material : SV_Target2;
  public float2 Velocity : SV_Target3;
}
