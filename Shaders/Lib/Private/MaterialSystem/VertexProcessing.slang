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

public struct InstanceData
{
  public Transform WorldTransform;
  public Transform PreviousWorldTransform;

  public Transform NormalTransform;

  public float3 _padding0;
  public uint   MaterialIndex;
}

/// Process the vertex input stream into the world space.
/// \param input The vertex input stream.
/// \param view The rendering view.
/// \param instance The currently rendered instance.
/// \param material The material data of the rendered instance.
/// \param instanceId The rendered instance ID.
/// \param isFrontFace Whether the geometry faces have front-facing normal vectors.
/// \return A transient data suitable as an input to a pixel shader.
public TransientStream ProcessWorldSpace(
  in InputStream   input,
  in PerViewBuffer view,
  in InstanceData  instance,
  in MaterialData  material,
  in uint          instanceId,
  in bool          isFrontFace
)
{
  TransientStream output;

  // Set the instance ID
  output.InstanceID  = instanceId;
  output.IsFrontFace = isFrontFace;

  // Normal/Tangent/BiTangent
#ifdef USE_NORMAL
  matrix normalMatrix = TransformToMatrix(instance.NormalTransform);
  float3 normal       = unpack(input.Normal);

  output.Normal = normalize(mul((float3x3)normalMatrix, normal));

#  ifdef USE_TANGENT
  float3 tangent = unpack(input.Tangent.xyz);

#    ifdef USE_BITANGENT
  float3 bitangent = unpack(input.BiTangent.xyz);
#    else
  float  magnitude = unpack(input.Tangent.w);
  float3 bitangent = cross(normal, tangent) * magnitude;
#    endif

  output.Tangent   = normalize(mul((float3x3)normalMatrix, tangent));
  output.BiTangent = normalize(mul((float3x3)normalMatrix, bitangent));
#  endif
#endif

  // Position
  matrix worldMatrix   = TransformToMatrix(instance.WorldTransform);
  float4 worldPosition = mul(worldMatrix, float4(input.Position, 1.0));

#ifdef USE_WORLDPOS
  output.WorldPosition = worldPosition.xyz;
#endif

  output.Position = mul(view.ViewProjectionMatrix, worldPosition);

#ifdef USE_VELOCITY
  matrix preiousWorldMatrix    = TransformToMatrix(instance.PreviousWorldTransform);
  float4 previousWorldPosition = mul(preiousWorldMatrix, float4(input.Position, 1.0));

#  ifdef USE_WORLDPOS
  output.PreviousWorldPosition = previousWorldPosition.xyz;
#  endif

  output.PreviousPosition = mul(view.PreviousViewProjectionMatrix, previousWorldPosition);
#endif

  // Texture Coordinates
#ifdef USE_TEXCOORD0
  output.TexCoord0 = float2(input.TexCoord0.x * material.UVTiling.x + material.UVOffset.x, input.TexCoord0.y * material.UVTiling.y + material.UVOffset.y);
#  ifdef USE_TEXCOORD1
  output.TexCoord1 = float2(input.TexCoord1.x * material.UVTiling.x + material.UVOffset.x, input.TexCoord1.y * material.UVTiling.y + material.UVOffset.y);
#  endif
#endif

  if (o_ReverseDepthRange)
    output.Position.z = 1.0f - output.Position.z;

  return output;
}
