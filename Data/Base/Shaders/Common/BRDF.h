#pragma once

#include <Shaders/Materials/MaterialData.h>

// TODO:
// check roughness^2 vs. roughness^4
// try different visibility function (SmithGGXCorrelated)

struct AccumulatedLight
{
  float3 diffuseLight;
  float3 specularLight;
};

AccumulatedLight InitializeLight(float3 diff, float3 spec)
{
  AccumulatedLight result;
  result.diffuseLight = diff;
  result.specularLight = spec;
  return result;
}

void AccumulateLight(inout AccumulatedLight result, AccumulatedLight light)
{
  result.diffuseLight += light.diffuseLight;
  result.specularLight += light.specularLight;
}

void AccumulateLight(inout AccumulatedLight result, AccumulatedLight light, float3 color)
{
  result.diffuseLight += light.diffuseLight * color;
  result.specularLight += light.specularLight * color;
}

float3 ComputeDiffuseEnergy(float3 F, float metalness)
{
  // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
  // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
  // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.

  float3 kS = F;         // The energy of light that gets reflected - Equal to Fresnel
  float3 kD = 1.0f - kS; // Remaining energy, light that gets refracted
  kD *= 1.0f - metalness; // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting

  return kD;
}

/*------------------------------------------------------------------------------
    Fresnel, visibility and normal distribution functions
------------------------------------------------------------------------------*/

float3 F_Schlick(const float3 f0, float f90, float NoH)
{
  // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
  return f0 + (f90 - f0) * pow(1.0 - NoH, 5.0);
}

float3 F_Schlick(const float3 f0, float NoH)
{
  float f = pow(1.0 - NoH, 5.0);
  return f + f0 * (1.0 - f);
}

float3 F_Schlick_Roughness(float3 f0, float NoH, float roughness)
{
  float3 a = 1.0 - roughness;
  return f0 + (max(a, f0) - f0) * pow(max(1.0 - NoH, 0.0), 5.0);
}

// Smith term for GGX
// [Smith 1967, "Geometrical shadowing of a random rough surface"]
inline float V_Smith(float a2, float NoV, float NoL)
{
  float Vis_SmithV = NoV + sqrt(NoV * (NoV - NoV * a2) + a2);
  float Vis_SmithL = NoL + sqrt(NoL * (NoL - NoL * a2) + a2);
  return rcp(Vis_SmithV * Vis_SmithL);
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
inline float V_SmithJointApprox(float a, float NoV, float NoL)
{
  float Vis_SmithV = NoL * (NoV * (1 - a) + a);
  float Vis_SmithL = NoV * (NoL * (1 - a) + a);
  return saturate_16(0.5 * rcp(Vis_SmithV + Vis_SmithL));
}

float V_GGX_anisotropic_2cos(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi)
{
  float cos2  = pow2(cos_theta_m);
  float sin2  = (1.0 - cos2);
  float s_x   = alpha_x * cos_phi;
  float s_y   = alpha_y * sin_phi;
  return 1.0 / max(cos_theta_m + sqrt(cos2 + (s_x * s_x + s_y * s_y) * sin2), 0.001);
}

// [Kelemen 2001, "A microfacet based coupled specular-matte brdf model with importance sampling"]
float V_Kelemen(float NoH)
{
  // constant to prevent NaN
  return rcp(4 * NoH * NoH + 1e-5);
}

// Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
float V_Neubelt(float NoV, float NoL)
{
  return saturate_16(1.0 / (4.0 * (NoL + NoV - NoL * NoV)));
}

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float a2, float NoH)
{
  float d = (NoH * a2 - NoH) * NoH + 1; // 2 mad
  return a2 / (PI * d * d); // 4 mul, 1 rcp
}

float D_GGX_Anisotropic(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi)
{
  float cos2  = pow2(cos_theta_m);
  float sin2  = (1.0 - cos2);
  float r_x   = cos_phi / alpha_x;
  float r_y   = sin_phi / alpha_y;
  float d     = cos2 + sin2 * (r_x * r_x + r_y * r_y);
  return saturate_16(1.0 / (PI * alpha_x * alpha_y * d * d));
}

float D_Charlie(float roughness, float NoH)
{
  // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
  float invAlpha  = 1.0 / roughness;
  float cos2h     = NoH * NoH;
  float sin2h     = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
  return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

/*------------------------------------------------------------------------------
    Diffuse
------------------------------------------------------------------------------*/

float3 Diffuse_Lambert(float3 diffuse_color)
{
  return diffuse_color * (1 / PI);
}

// [Burley 2012, "Physically-Based Shading at Disney"]
float3 Diffuse_Burley(ezMaterialData matData, float NoV, float NoL, float VoH)
{
  float FD90 = 0.5 + 2 * VoH * VoH * matData.roughness;
  float FdV = 1 + (FD90 - 1) * pow(1 - NoV, 5);
  float FdL = 1 + (FD90 - 1) * pow(1 - NoL, 5);
  return matData.diffuseColor * ( (1 / PI) * FdV * FdL );
}

// Diffuse - [Gotanda 2012, "Beyond a Simple Physically Based Blinn-Phong Model in Real-Time"]
float3 Diffuse_OrenNayar(ezMaterialData matData, float NoV, float NoL, float VoH)
{
  float a     = pow2(matData.roughness);
  float s     = a;                    // ( 1.29 + 0.5 * a );
  float s2    = s * s;
  float VoL   = 2 * VoH * VoH - 1;    // double angle identity
  float Cosri = VoL - NoV * NoL;
  float C1    = 1 - 0.5 * s2 / (s2 + 0.33);
  float C2    = 0.45 * s2 / (s2 + 0.09) * Cosri * ( Cosri >= 0 ? rcp( max( NoL, NoV + 0.0001f ) ) : 1 );
  return matData.diffuseColor / PI * ( C1 + C2 ) * ( 1 + matData.roughness * 0.5 );
}

float3 BRDF_Diffuse(ezMaterialData matData, float NoV, float NoL, float VoH)
{
  return Diffuse_OrenNayar(matData, NoV, NoL, VoH);
}

/*------------------------------------------------------------------------------
    Specular
------------------------------------------------------------------------------*/

float3 BRDF_Specular_Isotropic(
  ezMaterialData matData,
  float NoV,
  float NoL,
  float NoH,
  float VoH,
  float LoH,
  inout float3 kD,
  inout float3 kS
)
{
  float a  = pow2(matData.perceptualRoughness);
  float a2 = pow4(matData.perceptualRoughness);

  float V  = V_SmithJointApprox(a, NoV, NoL);
  float D  = D_GGX(a2, NoH);
  float3 F = F_Schlick(matData.specularColor, VoH);

  kD  *= ComputeDiffuseEnergy(F, matData.metalness);
  kS  *= F;

  return D * V * F;
}

inline float3 BRDF_Specular_Anisotropic(ezMaterialData matData, float3 v, float3 l, float3 h, float NoV, float NoL, float NoH, float LoH, inout float3 kD, inout float3 kS)
{
  // TODO: Add anisotropic brdf parameters in materials
  // // Construct TBN from the normal
  // float3 t, b;
  // find_best_axis_vectors(matData.normal, t, b);
  // float3x3 TBN = float3x3(t, b, matData.normal);

  // // Rotate tangent and bitagent
  // float rotation   = max(matData.anisotropic_rotation * PI2, FLT_MIN); // convert material property to a full rotation
  // float2 direction = float2(cos(rotation), sin(rotation));             // convert rotation to direction
  // t                = normalize(mul(float3(direction, 0.0f), TBN).xyz); // compute direction derived tangent
  // b                = normalize(cross(matData.normal, t));              // re-compute bitangent

  // float alpha_ggx = matData.roughness;
  // float aspect    = sqrt(1.0 - matData.anisotropic * 0.9);
  // float ax        = alpha_ggx / aspect;
  // float ay        = alpha_ggx * aspect;
  // float XdotH     = dot(t, h);
  // float YdotH     = dot(b, h);

  // // specular anisotropic BRDF
  // float D   = D_GGX_Anisotropic(NoH, ax, ay, XdotH, YdotH);
  // float V   = V_GGX_anisotropic_2cos(NoV, ax, ay, XdotH, YdotH) * V_GGX_anisotropic_2cos(NoV, ax, ay, XdotH, YdotH);
  // float f90 = saturate(dot(matData.F0, 50.0 * 0.33));
  // float3 F  = F_Schlick(matData.F0, f90, LoH);

  // kD  *= ComputeDiffuseEnergy(F, matData.metalness);
  // kS *= F;

  // return D * V * F;

  return float3(0.0f, 0.0f, 0.0f);
}

inline float3 BRDF_Specular_Clearcoat(ezMaterialData matData, float NoH, float VoH, inout float3 kD, inout float3 kS)
{
  // TODO: Add clearcoat brdf parameters in materials
  // float a2 = pow4(matData.roughness);

  // float D  = D_GGX(a2, NoH);
  // float V  = V_Kelemen(VoH);
  // float3 F = F_Schlick(0.04, 1.0, VoH) * matData.clearcoat;

  // kD  *= ComputeDiffuseEnergy(F, matData.metalness);
  // kS *= F;

  // return D * V * F;

  return float3(0.0f, 0.0f, 0.0f);
}

inline float3 BRDF_Specular_Sheen(ezMaterialData matData, float NoV, float NoL, float NoH, inout float3 kD, inout float3 kS)
{
  // TODO: Add sheen brdf parameters in materials
  // // Mix between white and using base color for sheen reflection
  // float tint = matData.sheen_tint * matData.sheen_tint;
  // float3 f0  = lerp(1.0f, matData.F0, tint);

  // float D  = D_Charlie(matData.roughness, NoH);
  // float V  = V_Neubelt(NoV, NoL);
  // float3 F = f0 * matData.sheen;

  // kD  *= ComputeDiffuseEnergy(F, matData.metalness);
  // kS *= F;

  // return D * V * F;

  return float3(0.0f, 0.0f, 0.0f);
}

///////////////////////////////////////////////////////////////////////////////////

float RoughnessFromMipLevel(uint mipLevel, uint mipCount)
{
  return pow(mipLevel / (float)(mipCount - 1), 2.0f);
}

float MipLevelFromRoughness(float roughness, uint mipCount)
{
  return (mipCount - 1) * sqrt(roughness);
}

float RoughnessFromPerceptualRoughness(in float roughness, in float3 normal, const float strength = 1.0f)
{
  static const float max_roughness_gain = 0.02f;

  float roughness2         = roughness * roughness;
  float3 dndu              = ddx(normal);
  float3 dndv              = ddy(normal);
  float variance           = (dot(dndu, dndu) + dot(dndv, dndv));
  float kernelRoughness2   = min(variance * strength, max_roughness_gain);
  float filteredRoughness2 = saturate(roughness2 + kernelRoughness2);

  return filteredRoughness2;
}

float PerceptualRoughnessFromRoughness(float roughness)
{
  return fast_sqrt(roughness);
}

float3 DiffuseLambert( float3 diffuseColor )
{
  return diffuseColor;
}

// divide by PI postponed
float SpecularGGX( float roughness, float NoH )
{
  // mad friendly reformulation of:
  //
  //              a^2
  // --------------------------------
  // PI * ((N.H)^2 * (a^2 - 1) + 1)^2

  float a2 = roughness * roughness;
  float f = ( NoH * a2 - NoH ) * NoH + 1.0f;
  return a2 / ( f * f );
}

float VisibilitySmithCorrelated( float roughness, float NdotV, float NdotL )
{
  float a2 = roughness * roughness;
  float lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
  float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
  return 0.5f / ( lambdaV + lambdaL );
}

float VisibilitySmithJointApprox( float roughness, float NdotV, float NdotL )
{
  float a = roughness;
  float b = 1.0f - a;
  float lambdaV = NdotL * ( NdotV * b + a );
  float lambdaL = NdotV * ( NdotL * b + a );
  return 0.5f / ( lambdaV + lambdaL );
}

float3 FresnelSchlick( float3 specularColor, float u )
{
  float f = 1.0f - u;
  float ff = f * f;
  float f5 = ff * ff * f;

  // specularColor below 2% is considered to be shadowing
  return saturate(50.0 * GetLuminance(specularColor)) * f5 + (1 - f5) * specularColor;
}

// note that 1/PI is applied later
AccumulatedLight DefaultShading(ezMaterialData matData, float3 L, float3 V)
{
  float3 N = matData.worldNormal;
  float3 H = normalize(L + V);

  float NoV = saturate(dot(N, V));
  float NoL = saturate(dot(N, L));
  float NoH = saturate(dot(N, H));

  float VoH = saturate(dot(V, H));
  float LoH = saturate(dot(L, H));

  float3 kS = 1.0f;
  float3 kD = 1.0f;

  float3 diffuse  = BRDF_Diffuse(matData, NoV, NoL, VoH);
  float3 specular = BRDF_Specular_Isotropic(matData, NoV, NoL, NoH, VoH, LoH, kD, kS);

  return InitializeLight(diffuse * kD * NoL, specular * NoL);
}

AccumulatedLight SubsurfaceShading(ezMaterialData matData, float3 L, float3 V)
{
  float3 N = matData.worldNormal;
  float3 H = normalize(V + L);

  float3 distortedLightDir = L + N * 0.1;
  float inScatter = pow(saturate(dot(V, -distortedLightDir)), matData.subsurfaceScatterPower);

  float wrapFactor = 0.5;
  float wrappedNdotH = saturate( ( dot(N, H) * wrapFactor + 1 - wrapFactor )) / (PI * 2);

	return InitializeLight(matData.subsurfaceColor * lerp(wrappedNdotH, 1, inScatter), 0.0);
}
