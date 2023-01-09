#pragma once

#include "Platforms.h"

// Common sampler states
SamplerState LinearSampler;
SamplerState LinearClampSampler;
SamplerState PointSampler;
SamplerState PointClampSampler;

static const float PI                  = 3.14159265f;
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
static const float RPC_9               = 0.11111111111f;
static const float RPC_16              = 0.0625f;
static const float ENVIRONMENT_MAX_MIP = 11.0f;

/*------------------------------------------------------------------------------
  MACROS
------------------------------------------------------------------------------*/
#define TexelSize ViewportSize.zw
#define TargetTexelSize TargetViewportSize.zw
#define degamma(color) pow(abs(color), Gamma)
#define gamma(color) pow(abs(color), 1.0f / Gamma)

/*------------------------------------------------------------------------------
  MATH
------------------------------------------------------------------------------*/

float min2(float2 value)
{
  return min(value.x, value.y);
}

float min3(float3 value)
{
  return min(min(value.x, value.y), value.z);
}

float min3(float a, float b, float c)
{
  return min(min(a, b), c);
}

float min4(float a, float b, float c, float d)
{
  return min(min(min(a, b), c), d);
}

float min5(float a, float b, float c, float d, float e)
{
  return min(min(min(min(a, b), c), d), e);
}

float max2(float2 value)
{
  return max(value.x, value.y);
}

float max3(float3 value)
{
  return max(max(value.x, value.y), value.z);
}

float max4(float a, float b, float c, float d)
{
  return max(max(max(a, b), c), d);
}

float max5(float a, float b, float c, float d, float e)
{
  return max(max(max(max(a, b), c), d), e);
}

float pow2(float x)
{
  return x * x;
}

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

bool is_saturated(float value)
{
  return value == saturate(value);
}

bool is_saturated(float2 value)
{
  return is_saturated(value.x) && is_saturated(value.y);
}

bool is_saturated(float3 value)
{
  return is_saturated(value.x) && is_saturated(value.y) && is_saturated(value.z);
}

bool is_saturated(float4 value)
{
  return is_saturated(value.x) && is_saturated(value.y) && is_saturated(value.z) && is_saturated(value.w);
}

bool is_valid_uv(float2 value)
{
  return (value.x >= 0.0f && value.x <= 1.0f) || (value.y >= 0.0f && value.y <= 1.0f);
}

/*------------------------------------------------------------------------------
    SATURATE
------------------------------------------------------------------------------*/
float saturate_11(float x)
{
  return clamp(x, FLT_MIN, FLT_MAX_11);
}

float2 saturate_11(float2 x)
{
  return clamp(x, FLT_MIN, FLT_MAX_11);
}

float3 saturate_11(float3 x)
{
  return clamp(x, FLT_MIN, FLT_MAX_11);
}

float4 saturate_11(float4 x)
{
  return clamp(x, FLT_MIN, FLT_MAX_11);
}

float saturate_16(float x)
{
  return clamp(x, FLT_MIN, FLT_MAX_16);
}

float2 saturate_16(float2 x)
{
  return clamp(x, FLT_MIN, FLT_MAX_16);
}

float3 saturate_16(float3 x)
{
  return clamp(x, FLT_MIN, FLT_MAX_16);
}

float4 saturate_16(float4 x)
{
  return clamp(x, FLT_MIN, FLT_MAX_16);
}

/*------------------------------------------------------------------------------
    PACKING/UNPACKING
------------------------------------------------------------------------------*/
float3 unpack(float3 value)
{
  return value * 2.0f - 1.0f;
}

float3 pack(float3 value)
{
  return value * 0.5f + 0.5f;
}

float2 unpack(float2 value)
{
  return value * 2.0f - 1.0f;
}

float2 pack(float2 value)
{
  return value * 0.5f + 0.5f;
}

float unpack(float value)
{
  return value * 2.0f - 1.0f;
}

float pack(float value)
{
  return value * 0.5f + 0.5f;
}

float pack_uint32_to_float16(uint i)
{
  return (float)i / FLT_MAX_16;
}

uint unpack_float16_to_uint32(float f)
{
  return round(f * FLT_MAX_16);
}

float pack_float_int(float f, uint i, uint numBitI, uint numBitTarget)
{
  // Constant optimize by compiler
  float precision = float(1U << numBitTarget);
  float maxi = float(1U << numBitI);
  float precisionMinusOne = precision - 1.0;
  float t1 = ((precision / maxi) - 1.0) / precisionMinusOne;
  float t2 = (precision / maxi) / precisionMinusOne;

  // Code
  return t1 * f + t2 * float(i);
}

void unpack_float_int(float val, uint numBitI, uint numBitTarget, out float f, out uint i)
{
  // Constant optimize by compiler
  float precision = float(1U << numBitTarget);
  float maxi = float(1U << numBitI);
  float precisionMinusOne = precision - 1.0;
  float t1 = ((precision / maxi) - 1.0) / precisionMinusOne;
  float t2 = (precision / maxi) / precisionMinusOne;

  // Code
  // extract integer part
  // + rcp(precisionMinusOne) to deal with precision issue
  i = int((val / t2) + rcp(precisionMinusOne));
  // Now that we have i, solve formula in PackFloatInt for f
  // f = (val - t2 * float(i)) / t1 => convert in mads form
  f = saturate((-t2 * float(i) + val) / t1); // Saturate in case of precision issue
}

/*------------------------------------------------------------------------------
    FAST MATH APPROXIMATIONS
------------------------------------------------------------------------------*/

// Relative error : < 0.7% over full
// Precise format : ~small float
// 1 ALU
float fast_sqrt(float x)
{
  int i = asint(x);
  i = 0x1FBD1DF5 + (i >> 1);
  return asfloat(i);
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
  y = P * (y * abs(y) - y) + y;
  return y;
}

float fast_cos(float x)
{
  return abs(abs(x) / PI2 % 4 - 2) - 1;
}

float4 RGBA8ToFloat4(uint x)
{
  float4 result;
  result.r = x & 0xFF;
  result.g = (x >> 8)  & 0xFF;
  result.b = (x >> 16) & 0xFF;
  result.a = (x >> 24) & 0xFF;

  return result / 255.0;
}

// for when the input data is already float4
float4 RGBA8ToFloat4(float4 x)
{
  return x;
}

float3 RGB8ToFloat3(uint x)
{
  float3 result;
  result.r = x & 0xFF;
  result.g = (x >> 8)  & 0xFF;
  result.b = (x >> 16) & 0xFF;

  return result / 255.0;
}

float3 RGB10ToFloat3(uint x)
{
  float3 result;
  result.r = x & 0x3FF;
  result.g = (x >> 10) & 0x3FF;
  result.b = (x >> 20) & 0x3FF;

  return result / 1023.0;
}

float2 RG16FToFloat2(uint x)
{
  float2 result;
  result.r = f16tof32(x);
  result.g = f16tof32(x >> 16);

  return result;
}

float4 RGBA16FToFloat4(uint rg, uint ba)
{
  return float4(RG16FToFloat2(rg), RG16FToFloat2(ba));
}

float GetLuminance(float3 color)
{
  return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

float GetLuminance(float4 color)
{
  return max(dot(color.rgb, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

float3 SrgbToLinear(float3 color)
{
  return (color < 0.04045) ? (color / 12.92) : pow(abs(color) / 1.055 + 0.0521327, 2.4);
}

float3 LinearToSrgb(float3 color)
{
  return (color < 0.0031308) ? (color * 12.92) : (1.055 * pow(abs(color), 1.0 / 2.4) - 0.055);
}

float3 CubeMapDirection(float3 inDirection)
{
  return float3(inDirection.x, inDirection.z, -inDirection.y);
}

float3 DecodeNormalTexture(float4 normalTex, in float intensity = 1.0f)
{
  // Avoid inverting normals
  intensity = max(intensity, 0.0f);

  float2 xy = unpack(normalTex.xy);
  float z = sqrt(max(1.0f - dot(xy, xy), 0.0));

  return float3(xy * intensity, z);
}

float InterleavedGradientNoise(float2 screenSpacePosition)
{
  float3 magic = float3(0.06711056, 0.00583715, 52.9829189);
  return frac(magic.z * frac(dot(screenSpacePosition, magic.xy)));
}

float3 NormalizeAndGetLength(float3 v, out float len)
{
  float squaredLen = dot(v, v);
  float reciprocalLen = rsqrt(squaredLen);
  len = squaredLen * reciprocalLen;
  return v * reciprocalLen;
}

float Square(float x)
{
  return x * x;
}
float2 Square(float2 x)
{
  return x * x;
}
float3 Square(float3 x)
{
  return x * x;
}
float4 Square(float4 x)
{
  return x * x;
}

float AdjustContrast(float value, float contrast)
{
  float a = -contrast;
  float b = contrast + 1;
  return saturate(lerp(a, b, value));
}

float3 Colorize(float3 baseColor, float3 color, float mask)
{
  return baseColor * lerp(1, 2 * color, mask);
}

float2 NDC2UV(float2 x)
{
  return x * float2(0.5f, -0.5f) + 0.5f;
}

float2 NDC2UV(float3 x)
{
  return x.xy * float2(0.5f, -0.5f) + 0.5f;
}

// Find good arbitrary axis vectors to represent U and V axes of a plane,
// given just the normal. Ported from UnMath.h
void FindBestAxisVectors(float3 In, out float3 Axis1, out float3 Axis2)
{
  const float3 N = abs(In);

  // Find best basis vectors.
  if (N.z > N.x && N.z > N.y)
  {
    Axis1 = float3(1, 0, 0);
  }
  else
  {
    Axis1 = float3(0, 0, 1);
  }

  Axis1 = normalize(Axis1 - In * dot(Axis1, In));
  Axis2 = cross(Axis1, In);
}
