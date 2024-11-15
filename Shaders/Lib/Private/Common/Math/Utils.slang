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

static const float PI              = 3.1415926535897932f;
static const float PI2             = 6.28318530f;
static const float PI4             = 12.5663706f;
static const float INV_PI          = 0.31830988f;
static const float PI_HALF         = PI * 0.5f;
static const float FLT_MIN         = 0.00000001f;
static const float FLT_MAX_10      = 511.0f;
static const float FLT_MAX_11      = 1023.0f;
static const float FLT_MAX_14      = 8191.0f;
static const float FLT_MAX_16      = 32767.0f;
static const float FLT_MAX_16U     = 65535.0f;
static const float ALPHA_THRESHOLD = 0.6f;
static const float RPC_9           = 0.11111111111f;
static const float RPC_16          = 0.0625f;

[ForceInline]
float min2(float2 value) { return min(value.x, value.y); }

[ForceInline]
float min3(float3 value) { return min(min(value.x, value.y), value.z); }

[ForceInline]
float min3(float a, float b, float c) { return min(min(a, b), c); }

[ForceInline]
float min4(float a, float b, float c, float d) { return min(min(min(a, b), c), d); }

[ForceInline]
float min5(float a, float b, float c, float d, float e) { return min(min(min(min(a, b), c), d), e); }

[ForceInline]
float max2(float2 value) { return max(value.x, value.y); }

[ForceInline]
float max3(float3 value) { return max(max(value.x, value.y), value.z); }

[ForceInline]
float max4(float a, float b, float c, float d) { return max(max(max(a, b), c), d); }

[ForceInline]
float max5(float a, float b, float c, float d, float e) { return max(max(max(max(a, b), c), d), e); }

[ForceInline]
float pow2(float x) { return x * x; }

float pow3(float x)
{
  float xx = x * x;
  return xx * x;
}

float pow4(float x)
{
  float xx = x * x;
  return xx * xx;
}

[ForceInline]
float saturate11(float x) { return clamp(x, FLT_MIN, FLT_MAX_11); }

[ForceInline]
float2 saturate11(float2 x) { return clamp(x, FLT_MIN, FLT_MAX_11); }

[ForceInline]
float3 saturate11(float3 x) { return clamp(x, FLT_MIN, FLT_MAX_11); }

[ForceInline]
float4 saturate11(float4 x) { return clamp(x, FLT_MIN, FLT_MAX_11); }

[ForceInline]
float saturate16(float x) { return clamp(x, FLT_MIN, FLT_MAX_16); }

[ForceInline]
float2 saturate16(float2 x) { return clamp(x, FLT_MIN, FLT_MAX_16); }

[ForceInline]
float3 saturate16(float3 x) { return clamp(x, FLT_MIN, FLT_MAX_16); }

[ForceInline]
float4 saturate16(float4 x) { return clamp(x, FLT_MIN, FLT_MAX_16); }

[ForceInline]
float3 unpack(float3 value) { return value * 2.0f - 1.0f; }

[ForceInline]
float3 pack(float3 value) { return value * 0.5f + 0.5f; }

[ForceInline]
float2 unpack(float2 value) { return value * 2.0f - 1.0f; }

[ForceInline]
float2 pack(float2 value) { return value * 0.5f + 0.5f; }

[ForceInline]
float unpack(float value) { return value * 2.0f - 1.0f; }

[ForceInline]
float pack(float value) { return value * 0.5f + 0.5f; }

// Relative error : < 0.7% over full
// Precise format : ~small float
// 1 ALU
float fast_sqrt(float x)
{
  int i = (int)x;
  i     = 0x1FBD1DF5 + (i >> 1);
  return (float)i;
}

float fast_length(float3 v)
{
  float LengthSqr = dot(v, v);
  return fast_sqrt(LengthSqr);
}

float fast_sin(float x)
{
  const float B = 4 / PI;
  const float C = -4 / PI2;
  const float P = 0.225;

  float y = B * x + C * x * abs(x);
  y       = P * (y * abs(y) - y) + y;
  return y;
}

[ForceInline]
float fast_cos(float x) { return abs(abs(x) / PI2 % 4 - 2) - 1; }

[ForceInline]
float random(float2 uv) { return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453); }

[ForceInline]
bool IsSaturated(float value) { return value == saturate(value); }

[ForceInline]
bool IsSaturated(float2 value) { return IsSaturated(value.x) && IsSaturated(value.y); }

[ForceInline]
bool IsSaturated(float3 value) { return IsSaturated(value.x) && IsSaturated(value.y) && IsSaturated(value.z); }

[ForceInline]
bool IsSaturated(float4 value) { return IsSaturated(value.x) && IsSaturated(value.y) && IsSaturated(value.z) && IsSaturated(value.w); }

[ForceInline]
bool IsValidUV(float2 value) { return (value.x >= 0.0f && value.x <= 1.0f) || (value.y >= 0.0f && value.y <= 1.0f); }

[ForceInline]
float2 ndc2uv(float2 x) { return x.xy * float2(0.5f, -0.5f) + 0.5f; }

[ForceInline]
float2 ndc2uv(float3 x) { return x.xy * float2(0.5f, -0.5f) + 0.5f; }

void FindBestAxisVectors(in float3 n, out float3 u, out float3 v)
{
    const float NX = abs(n.x);
    const float NY = abs(n.y);
    const float NZ = abs(n.z);

    // Find best basis vectors.
    if (NZ > NX && NZ > NY)	u = float3(1, 0, 0);
    else                    u = float3(0, 0, 1);

    u = normalize(u - n * (dot(u, n)));
    v = cross(u, n);
}