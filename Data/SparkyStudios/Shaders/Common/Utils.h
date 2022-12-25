#pragma once

#include <Shaders/Common/Common.h>
#include <Shaders/Common/GlobalConstants.h>
#include <Shaders/Common/CameraConstantsAccess.h>

/*------------------------------------------------------------------------------
  CONSTANTS
------------------------------------------------------------------------------*/
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
static const float ENVIRONMENT_MAX_MIP = 11.0f;

/*------------------------------------------------------------------------------
  MACROS
------------------------------------------------------------------------------*/
#define TexelSize ViewportSize.zw
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

/*------------------------------------------------------------------------------
    TRANSFORMATIONS
------------------------------------------------------------------------------*/
float3 world_to_view(float3 x, bool is_position = true)
{
  return mul(float4(x, (float)is_position), GetWorldToCameraMatrix()).xyz;
}

float3 world_to_ndc(float3 x, bool is_position = true)
{
  float4 ndc = mul(float4(x, (float)is_position), GetWorldToScreenMatrix());
  return ndc.xyz / ndc.w;
}

float3 world_to_ndc(float3 x, float4x4 transform) // shadow mapping
{
  float4 ndc = mul(float4(x, 1.0f), transform);
  return ndc.xyz / ndc.w;
}

float3 view_to_ndc(float3 x, bool is_position = true)
{
  float4 ndc = mul(float4(x, (float)is_position), GetCameraToScreenMatrix());
  return ndc.xyz / ndc.w;
}

float2 world_to_uv(float3 x, bool is_position = true)
{
  float4 uv = mul(float4(x, (float)is_position), GetWorldToScreenMatrix());
  return (uv.xy / uv.w) * float2(0.5f, -0.5f) + 0.5f;
}

float2 view_to_uv(float3 x, bool is_position = true)
{
  float4 uv = mul(float4(x, (float)is_position), GetCameraToScreenMatrix());
  return (uv.xy / uv.w) * float2(0.5f, -0.5f) + 0.5f;
}

float2 ndc_to_uv(float2 x)
{
  return x * float2(0.5f, -0.5f) + 0.5f;
}

float2 ndc_to_uv(float3 x)
{
  return x.xy * float2(0.5f, -0.5f) + 0.5f;
}

float3 get_position_ws_from_depth(const float2 uv, const float depth)
{
  float x = uv.x * 2.0f - 1.0f;
  float y = (1.0f - uv.y) * 2.0f - 1.0f;
  float4 pos_clip = float4(x, y, depth, 1.0f);
  float4 pos_world = mul(pos_clip, GetScreenToWorldMatrix());
  return pos_world.xyz / pos_world.w;
}

/*------------------------------------------------------------------------------
    DIRECTION UV
------------------------------------------------------------------------------*/
float2 direction_sphere_uv(float3 direction)
{
  float n = length(direction.xz);
  float2 uv = float2((n > 0.0000001) ? direction.x / n : 0.0, direction.y);
  uv = acos(uv) * INV_PI;
  uv.x = (direction.z > 0.0) ? uv.x * 0.5 : 1.0 - (uv.x * 0.5);
  uv.x = 1.0 - uv.x;

  return uv;
}

uint direction_to_cube_face_index(const float3 direction)
{
  float3 direction_abs = abs(direction);
  float max_coordinate = max3(direction_abs);

  if (max_coordinate == direction_abs.x)
  {
    return direction_abs.x == direction.x ? 0 : 1;
  }
  else if (max_coordinate == direction_abs.y)
  {
    return direction_abs.y == direction.y ? 2 : 3;
  }
  else
  {
    return direction_abs.z == direction.z ? 4 : 5;
  }

  return 0;
}

/*------------------------------------------------------------------------------
    NOISE/OFFSETS/ROTATIONS
------------------------------------------------------------------------------*/
float get_random(float2 uv)
{
  return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

// Based on Activision GTAO paper: https://www.activision.com/cdn/research/s2016_pbs_activision_occlusion.pptx
float get_offset_non_temporal(uint2 screen_pos)
{
  int2 position = (int2)(screen_pos);
  return 0.25 * (float)((position.y - position.x) & 3);
}

/*------------------------------------------------------------------------------
    OCCLUSION/SHADOWING
------------------------------------------------------------------------------*/
// The Technical Art of Uncharted 4 - http://advances.realtimerendering.com/other/2016/naughty_dog/index.html
float microw_shadowing_nt(float n_dot_l, float ao)
{
  float aperture = 2.0f * ao * ao;
  return saturate(abs(n_dot_l) + aperture - 1.0f);
}

// Chan 2018, "Material Advances in Call of Duty: WWII"
float microw_shadowing_cod(float n_dot_l, float visibility)
{
  float aperture = rsqrt(1.0 - visibility);
  float microShadow = saturate(n_dot_l * aperture);
  return microShadow * microShadow;
}

/*------------------------------------------------------------------------------
    PRIMITIVES
------------------------------------------------------------------------------*/
float draw_line(float2 p1, float2 p2, float2 uv, float a)
{
  float r = 0.0f;
  float one_px = 1. / ViewportSize.x; // not really one px

  // get dist between points
  float d = distance(p1, p2);

  // get dist between current pixel and p1
  float duv = distance(p1, uv);

  // if point is on line, according to dist, it should match current uv
  r = 1.0f - floor(1.0f - (a * one_px) + distance(lerp(p1, p2, clamp(duv / d, 0.0f, 1.0f)), uv));

  return r;
}

float draw_line_view_space(float3 p1, float3 p2, float2 uv, float a)
{
  float2 p1_uv = view_to_uv(p1);
  float2 p2_uv = view_to_uv(p2);
  return draw_line(p1_uv, p2_uv, uv, a);
}

float draw_circle(float2 origin, float radius, float2 uv)
{
  return (distance(origin, uv) <= radius) ? 1.0f : 0.0f;
}

float draw_circle_view_space(float3 origin, float radius, float2 uv)
{
  float2 origin_uv = view_to_uv(origin);
  return draw_circle(origin_uv, radius, uv);
}

/*------------------------------------------------------------------------------
    MISC
------------------------------------------------------------------------------*/
float3 compute_diffuse_energy(float3 F, float metallic)
{
  float3 kS = F;         // The energy of light that gets reflected - Equal to Fresnel
  float3 kD = 1.0f - kS; // Remaining energy, light that gets refracted
  kD *= 1.0f - metallic; // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting

  return kD;
}

float screen_fade(float2 uv)
{
  float2 fade = max(0.0f, 12.0f * abs(uv - 0.5f) - 5.0f);
  return saturate(1.0f - dot(fade, fade));
}

// Find good arbitrary axis vectors to represent U and V axes of a plane,
// given just the normal. Ported from UnMath.h
void find_best_axis_vectors(float3 In, out float3 Axis1, out float3 Axis2)
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

// http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
float3 dither(uint2 screen_pos)
{
  float3 dither = dot(float2(171.0f, 231.0f), float2(screen_pos));
  dither        = frac(dither / float3(103.0f, 71.0f, 97.0f));
  dither        /= 255.0f;

  return dither;
}

static const float3 hemisphere_samples[64] =
  {
    float3(0.04977, -0.04471, 0.04996),
    float3(0.01457, 0.01653, 0.00224),
    float3(-0.04065, -0.01937, 0.03193),
    float3(0.01378, -0.09158, 0.04092),
    float3(0.05599, 0.05979, 0.05766),
    float3(0.09227, 0.04428, 0.01545),
    float3(-0.00204, -0.0544, 0.06674),
    float3(-0.00033, -0.00019, 0.00037),
    float3(0.05004, -0.04665, 0.02538),
    float3(0.03813, 0.0314, 0.03287),
    float3(-0.03188, 0.02046, 0.02251),
    float3(0.0557, -0.03697, 0.05449),
    float3(0.05737, -0.02254, 0.07554),
    float3(-0.01609, -0.00377, 0.05547),
    float3(-0.02503, -0.02483, 0.02495),
    float3(-0.03369, 0.02139, 0.0254),
    float3(-0.01753, 0.01439, 0.00535),
    float3(0.07336, 0.11205, 0.01101),
    float3(-0.04406, -0.09028, 0.08368),
    float3(-0.08328, -0.00168, 0.08499),
    float3(-0.01041, -0.03287, 0.01927),
    float3(0.00321, -0.00488, 0.00416),
    float3(-0.00738, -0.06583, 0.0674),
    float3(0.09414, -0.008, 0.14335),
    float3(0.07683, 0.12697, 0.107),
    float3(0.00039, 0.00045, 0.0003),
    float3(-0.10479, 0.06544, 0.10174),
    float3(-0.00445, -0.11964, 0.1619),
    float3(-0.07455, 0.03445, 0.22414),
    float3(-0.00276, 0.00308, 0.00292),
    float3(-0.10851, 0.14234, 0.16644),
    float3(0.04688, 0.10364, 0.05958),
    float3(0.13457, -0.02251, 0.13051),
    float3(-0.16449, -0.15564, 0.12454),
    float3(-0.18767, -0.20883, 0.05777),
    float3(-0.04372, 0.08693, 0.0748),
    float3(-0.00256, -0.002, 0.00407),
    float3(-0.0967, -0.18226, 0.29949),
    float3(-0.22577, 0.31606, 0.08916),
    float3(-0.02751, 0.28719, 0.31718),
    float3(0.20722, -0.27084, 0.11013),
    float3(0.0549, 0.10434, 0.32311),
    float3(-0.13086, 0.11929, 0.28022),
    float3(0.15404, -0.06537, 0.22984),
    float3(0.05294, -0.22787, 0.14848),
    float3(-0.18731, -0.04022, 0.01593),
    float3(0.14184, 0.04716, 0.13485),
    float3(-0.04427, 0.05562, 0.05586),
    float3(-0.02358, -0.08097, 0.21913),
    float3(-0.14215, 0.19807, 0.00519),
    float3(0.15865, 0.23046, 0.04372),
    float3(0.03004, 0.38183, 0.16383),
    float3(0.08301, -0.30966, 0.06741),
    float3(0.22695, -0.23535, 0.19367),
    float3(0.38129, 0.33204, 0.52949),
    float3(-0.55627, 0.29472, 0.3011),
    float3(0.42449, 0.00565, 0.11758),
    float3(0.3665, 0.00359, 0.0857),
    float3(0.32902, 0.0309, 0.1785),
    float3(-0.08294, 0.51285, 0.05656),
    float3(0.86736, -0.00273, 0.10014),
    float3(0.45574, -0.77201, 0.00384),
    float3(0.41729, -0.15485, 0.46251),
    float3(-0.44272, -0.67928, 0.1865)};
