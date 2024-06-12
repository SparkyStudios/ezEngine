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

#include <Common/Constants.slang>
#include <Common/RenderingContext.slang>

float4 ClipToView(in matrix inverseProjectionMatrix, in float4 clip)
{
  // View space transform
  float4 view = mul(inverseProjectionMatrix, clip);

  // Perspective projection
  view = view / view.w;

  return view;
}

float4 ScreenToView(in matrix inverseProjectionMatrix, in float4 screen, in float2 renderSize)
{
  // Convert to NDC
  float2 texCoord = screen.xy / renderSize.xy;

  if (o_IsClipSpaceYInverted)
    texCoord.y = 1.0f - texCoord.y;

  // Convert to clip space
  float4 clip = float4(texCoord.xy * 2.0f - 1.0f, screen.z, screen.w);

  return ClipToView(inverseProjectionMatrix, clip);
}
