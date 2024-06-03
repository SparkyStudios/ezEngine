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

#include <Common/Math/Transform.slang>
#include <Materials/Material.slang>
#include <Materials/Streams.slang>
#include <RenderingContext.slang>

struct PerInstanceData
{
  Transform WorldTransform;
  Transform PreviousWorldTransform;

  Transform NormalTransform;
}

[[vk_binding(3, 0)]]
StructuredBuffer<PerInstanceData> Buffer_PerInstance : register(b3, space0);

/// \brief Gets the instance data for the given instance ID.
/// \param instanceID The ID of the instance.
/// \return The instance data.
func GetInstanceData(instanceID: uint) -> PerInstanceData
{
  return Buffer_PerInstance[instanceID];
}

/// \brief Gets the previous transform of the currently drawn object.
matrix GetPreviousTransform() { return Buffer_PerPass.Values; }

TransientStream ProcessWorldSpace(InputStream input)
{
  TransientStream output;

#ifdef USE_TEXCOORD0
  Material material = GetMaterial();
  output.TexCoord0 = float2(input.TexCoord0.x * material.UVTiling.x + material.UVOffset.x, input.TexCoord0.y * material.UVTiling.y + material.UVOffset.y);
#ifdef USE_TEXCOORD1
  output.TexCoord1 = float2(input.TexCoord1.x * material.UVTiling.x + material.UVOffset.x, input.TexCoord1.y * material.UVTiling.y + material.UVOffset.y);
#endif
#endif

  return output;
}
