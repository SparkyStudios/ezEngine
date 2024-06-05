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

#include <Common.slang>
#include <Materials.slang>

ConstantBuffer<PerFrameBuffer> Buffer_PerFrame;

ConstantBuffer<PerViewBuffer> Buffer_PerView;

readonly StructuredBuffer<PerInstanceData> Buffer_PerInstance;

/// Gets the instance data for the given instance ID.
/// \param instanceID The ID of the instance.
/// \return The instance data.
func GetInstanceData(instanceID: uint) -> PerInstanceData { return Buffer_PerInstance[instanceID]; }

[shader("vertex")]
TransientStream VSMain(InputStream input, uint instanceID : SV_InstanceID, uint vertexID : SV_VertexID)
{
  return ProcessWorldSpace(
    input,
    Buffer_PerView,
    GetInstanceData(instanceID),
    instanceID,
    false, // TODO: Check how to get this value on Metal
  );
}

[shader("pixel")]
float4 PSMain(TransientStream input) : SV_Target0
{
  OutputStream output;

  output.Albedo = float4(input.Normal.xyz, 1.0f);

  return output.Albedo;
}
