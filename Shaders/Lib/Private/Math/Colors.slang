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

implementing Math;

extern static const float o_GammaSDR = 2.2f; // SDR monitors are likely old, and aim for a simplistic gamma 2.2 curve
extern static const float o_GammaHDR = 2.4f; // HDR monitors are more likely to aim for the actual sRGB standard, which has a curve that for mid tones to high lights resembles a gamma of 2.4

/// Unpacks an unsigned integer to an RGBA color value.
/// \param x The unsigned integer to unpack.
/// \return The unpacked color as a \c float4.
public float4 UnpackColor4(uint x)
{
  float4 result;
  result.r = x & 0xFF;
  result.g = (x >> 8) & 0xFF;
  result.b = (x >> 16) & 0xFF;
  result.a = (x >> 24) & 0xFF;

  return result / 255.0f;
}

/// Unpacks an unsigned integer to a 8-bits RGB color value.
/// \param x The unsigned integer to unpack.
/// \return The unpacked color as a \c float3.
public float3 UnpackColor3_8Bits(uint x)
{
  float3 result;
  result.r = x & 0xFF;
  result.g = (x >> 8) & 0xFF;
  result.b = (x >> 16) & 0xFF;

  return result / 255.0f;
}

/// Unpacks an unsigned integer to a 10-bits RGB color value.
/// \param x The unsigned integer to unpack.
/// \return The unpacked color as a \c float3
public float3 UnpackColor3_10Bits(uint x)
{
  float3 result;
  result.r = x & 0x3FF;
  result.g = (x >> 10) & 0x3FF;
  result.b = (x >> 20) & 0x3FF;

  return result / 1023.0f;
}

/// Unpacks an unsigned integer to a 16-bits RG color value.
/// \param x The unsigned integer to unpack.
/// \return The unpacked color as a \c float2.
public float2 UnpackColor2_16Bits(uint x)
{
  float2 result;
  result.r = f16tof32(x);
  result.g = f16tof32(x >> 16);

  return result;
}

/// Unpacks to 16-bit colors (RG and BA) packed into two
/// unsigned integers to a single RGBA color value
/// \param rg The packed RG values.
/// \param ba The packed BA values.
/// \return The unpacked color as a \c float4.
public float4 UnpackColor4(uint rg, uint ba) { return float4(UnpackColor2_16Bits(rg), UnpackColor2_16Bits(ba)); }

/// Gets the luminance of the given \a color
/// \param color The color to get the luminance from.
/// \return The color's luminance.
public float GetLuminance(float3 color) { return dot(color, float3(0.2126, 0.7152, 0.0722)); }

/// Converts the given \a color from sRGB color space
/// to Linear color space.
/// \param color The color to convert.
/// \param hdrEnabled Whether HDR support is enabled.
/// \return A new color in the Linear color space.
public float3 SrgbToLinear(float3 color, bool hdrEnabled)
{
  float  gamma       = hdrEnabled ? o_GammaHDR : o_GammaSDR;
  float3 linear_low  = color / 12.92;
  float3 linear_high = pow((color + 0.055) / 1.055, gamma);
  float3 is_high     = step(0.0404482362771082, color);
  return lerp(linear_low, linear_high, is_high);
}

/// Converts the given \a color from Linar color space
/// to sRGB color space.
/// \param color The color to convert.
/// \param hdrEnabled Whether HDR support is enabled.
/// \return A new color in the sRGB color space.
public float3 LinearToSrgb(float3 color, bool hdrEnabled)
{
  float  gamma     = hdrEnabled ? o_GammaHDR : o_GammaSDR;
  float3 srgb_low  = color * 12.92;
  float3 srgb_high = 1.055 * pow(color, 1.0 / gamma) - 0.055;
  float3 is_high   = step(0.00313066844250063, color);
  return lerp(srgb_low, srgb_high, is_high);
}

/// Converts the given \a color from Linear color space
/// to HDR10 color space.
/// \param color The color to convert.
/// \param whitePoint The HDR white point.
/// \return A new color in the HDR10 color space.
public float3 LinearToHDR10(float3 color, float whitePoint)
{
  // convert Rec.709 (similar to srgb) to Rec.2020 color space
  {
    static const float3x3 from709to2020 =
      {
        { 0.6274040f, 0.3292820f, 0.0433136f },
        { 0.0690970f, 0.9195400f, 0.0113612f },
        { 0.0163916f, 0.0880132f, 0.8955950f }
      };

    color = mul(from709to2020, color);
  }

  // normalize HDR scene values ([0..>1] to [0..1]) for the ST.2084 curve
  const float st2084_max = 10000.0f;
  color *= whitePoint / st2084_max;

  // apply ST.2084 (PQ curve) for HDR10 standard
  {
    static const float m1 = 2610.0 / 4096.0 / 4;
    static const float m2 = 2523.0 / 4096.0 * 128;
    static const float c1 = 3424.0 / 4096.0;
    static const float c2 = 2413.0 / 4096.0 * 32;
    static const float c3 = 2392.0 / 4096.0 * 32;
    float3             cp = pow(abs(color), m1);
    color                 = pow((c1 + c2 * cp) / (1 + c3 * cp), m2);
  }

  return color;
}
