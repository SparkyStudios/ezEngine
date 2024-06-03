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

static const float PI                  = 3.1415926535897932f;
static const float PI2                 = 6.28318530f;
static const float PI4                 = 12.5663706f;
static const float INV_PI              = 0.31830988f;
static const float PI_HALF             = PI * 0.5f;
static const float FLT_MIN             = 0.00000001f;
static const float FLT_MAX_10          = 511.0f;
static const float FLT_MAX_11          = 1023.0f;
static const float FLT_MAX_14          = 8191.0f;
static const float FLT_MAX_16          = 32767.0f;
static const float FLT_MAX_16U         = 65535.0f;
static const float ALPHA_THRESHOLD     = 0.6f;
static const float RPC_9               = 0.11111111111f;
static const float RPC_16              = 0.0625f;

func min2(value: float2) -> float { return min(value.x, value.y); }
func min3(value: float3) -> float { return min(min(value.x, value.y), value.z); }
func min3(a: float, b: float, c: float) -> float { return min(min(a, b), c); }
func min4(a: float, b: float, c: float, d: float) -> float { return min(min(min(a, b), c), d); }
func min5(a: float, b: float, c: float, d: float, e: float) -> float { return min(min(min(min(a, b), c), d), e); }
func max2(value: float2) -> float { return max(value.x, value.y); }
func max3(value: float3) -> float { return max(max(value.x, value.y), value.z); }
func max4(a: float, b: float, c: float, d: float) -> float { return max(max(max(a, b), c), d); }
func max5(a: float, b: float, c: float, d: float, e: float) -> float { return max(max(max(max(a, b), c), d), e); }

func pow2(x: float) -> float
{
  return x * x;
}

func pow3(x: float) -> float
{
  float xx = x * x;
  return xx * x;
}

func pow4(x: float) -> float
{
  float xx = x * x;
  return xx * xx;
}

func saturate11(x: float) -> float { return clamp(x, FLT_MIN, FLT_MAX_11); }
func saturate11(x: float2) -> float2 { return clamp(x, FLT_MIN, FLT_MAX_11); }
func saturate11(x: float3) -> float3 { return clamp(x, FLT_MIN, FLT_MAX_11); }
func saturate11(x: float4) -> float4 { return clamp(x, FLT_MIN, FLT_MAX_11); }
func saturate16(x: float) -> float { return clamp(x, FLT_MIN, FLT_MAX_16); }
func saturate16(x: float2) -> float2 { return clamp(x, FLT_MIN, FLT_MAX_16); }
func saturate16(x: float3) -> float3 { return clamp(x, FLT_MIN, FLT_MAX_16); }
func saturate16(x: float4) -> float4 { return clamp(x, FLT_MIN, FLT_MAX_16); }

func unpack(value: float3) -> float3 { return value * 2.0f - 1.0f; }
func pack(value: float3) -> float3   { return value * 0.5f + 0.5f; }
func unpack(value: float2) -> float2 { return value * 2.0f - 1.0f; }
func pack(value: float2) -> float2   { return value * 0.5f + 0.5f; }
func unpack(value: float) -> float   { return value * 2.0f - 1.0f; }
func pack(value: float) -> float     { return value * 0.5f + 0.5f; }

// Relative error : < 0.7% over full
// Precise format : ~small float
// 1 ALU
func fast_sqrt(x: float) -> float
{
  int i = (int)x;
  i = 0x1FBD1DF5 + (i >> 1);
  return (float)i;
}

func fast_length(v: float3) -> float
{
  float LengthSqr = dot(v, v);
  return fast_sqrt(LengthSqr);
}

func fast_sin(x: float) -> float
{
  const float B = 4 / PI;
  const float C = -4 / PI2;
  const float P = 0.225;

  float y = B * x + C * x * abs(x);
  y = P * (y * abs(y) - y) + y;
  return y;
}

func fast_cos(x: float) -> float { return abs(abs(x) / PI2 % 4 - 2) - 1; }

func random(uv: float2) -> float { return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453); }

func IsSaturated(value: float) -> bool { return value == saturate(value); }

func IsSaturated(value: float2) -> bool { return IsSaturated(value.x) && IsSaturated(value.y); }

func IsSaturated(value: float3) -> bool { return IsSaturated(value.x) && IsSaturated(value.y) && IsSaturated(value.z); }

func IsSaturated(value: float4) -> bool { return IsSaturated(value.x) && IsSaturated(value.y) && IsSaturated(value.z) && IsSaturated(value.w); }

func IsValidUV(value: float2) -> bool { return (value.x >= 0.0f && value.x <= 1.0f) || (value.y >= 0.0f && value.y <= 1.0f); }
