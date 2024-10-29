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

#include <Common.slang>
#include <Materials.slang>

struct Params
{
  /// Buffer containing data updated per frames.
  ParameterBlock<PerFrameBuffer> Buffer_PerFrame;

  /// Buffer containing data updated per views.
  ParameterBlock<PerViewBuffer> Buffer_PerView;

  /// Buffer containing data per mesh instances.
  StructuredBuffer<InstanceData> Buffer_PerInstance;

  /// Buffer containing data per mesh materials.
  StructuredBuffer<MaterialData> Buffer_PerMaterial;
}

/// Shader parameters provided by this render stage.
ParameterBlock<Params> g_Params;

/// Gets the instance data for the given instance ID.
/// \param instanceID The ID of the instance.
/// \return The instance data.
[ForceInline]
InstanceData GetInstanceData(in uint instanceID) { return g_Params.Buffer_PerInstance[instanceID]; }

/// Gets the material data for the given material ID.
/// \param materialID The ID of the material.
/// \return The material data.
[ForceInline]
MaterialData GetMaterialData(in uint materialID) { return g_Params.Buffer_PerMaterial[materialID]; }

[shader("vertex")]
TransientStream VSMain(InputStream input, uint instanceID: SV_InstanceID, uint vertexID: SV_VertexID)
{
  const InstanceData instance = GetInstanceData(instanceID);
  const MaterialData material = GetMaterialData(instance.MaterialIndex);

  return ProcessWorldSpace(
    input,
    g_Params.Buffer_PerView,
    instance,
    material,
    instanceID,
    false, // TODO: Check how to get this value on Metal
  );
}

[shader("pixel")]
float4 PSMain<T : IMaterial>(TransientStream input)
  : SV_Target0
{
  /// Create the material shader instance
  T shader = T.create();

  OutputStream output;

  InstanceData instance = GetInstanceData(input.InstanceID);
  MaterialData material = GetMaterialData(instance.MaterialIndex);

  Surface surface = GenerateSurface(shader, input, material);

#ifdef USE_WORLDPOS
  if (surface.Opacity <= GetAlphaThreshold(input.WorldPosition, g_Params.Buffer_PerView.Position))
    discard;
#else
  if (surface.Opacity <= o_DefaultAlphaThreshold)
    discard;
#endif

#ifdef USE_VELOCITY
  // Convert to ndc
  float2 position         = (input.Position.xy / input.Position.w);
  float2 previousPosition = (input.PreviousPosition.xy / input.PreviousPosition.w);

  // Remove the ndc jitter
  // position  -= g_Params.Buffer_PerFrame.taa_jitter_current;
  // previousPosition -= g_Params.Buffer_PerFrame.taa_jitter_previous;

  // G-Buffer Velocity output
  output.Velocity = ndc2uv(position) - ndc2uv(previousPosition);
#endif

  // G-Buffer Diffuse output
  output.Diffuse = float4(
    surface.Diffuse + surface.Emissive,
    GetLuminance(surface.Emissive)
  );

  // G-Buffer Normal output
  output.Normal = float4(
    surface.WorldNormal,
    instance.MaterialIndex
  );

  // G-Buffer Material output
  output.Material = float4(
    surface.Metalness,
    surface.Roughness,
    surface.Occlusion,
    surface.Cavity
  );

  return output.Diffuse;
}
