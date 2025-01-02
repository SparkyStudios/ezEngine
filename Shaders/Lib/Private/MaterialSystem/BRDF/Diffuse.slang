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

implementing MaterialSystem;

public struct DiffuseLambert : IBRDF
{
  float3 albedo;

  __init(float3 albedo)
  {
    this.albedo = albedo;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo) { return albedo * INV_PI * angularInfo.NoL; }
}

// [Burley 2012, "Physically-Based Shading at Disney"]
public struct DiffuseBurley : IBRDF
{
  float3 albedo;
  float  roughness;

  __init(float3 albedo, float linearRoughness)
  {
    this.albedo    = albedo;
    this.roughness = linearRoughness;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    float FD90 = 0.5 + 2 * angularInfo.VoH * angularInfo.VoH * roughness;
    float FdV  = 1 + (FD90 - 1) * pow(1 - angularInfo.NoV, 5);
    float FdL  = 1 + (FD90 - 1) * pow(1 - angularInfo.NoL, 5);

    return albedo * (INV_PI * FdV * FdL) * angularInfo.NoL;
  }
}

// Diffuse - [Gotanda 2012, "Beyond a Simple Physically Based Blinn-Phong Model in Real-Time"]
public struct DiffuseOrenNayar : IBRDF
{
  private float3 albedo;
  private float  roughness;
  private float  roughnessA;

  __init(Surface surface)
  {
    this.albedo     = surface.Diffuse;
    this.roughness  = surface.LinearRoughness;
    this.roughnessA = surface.Roughness;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    float a     = roughnessA;
    float s     = a;                                         // ( 1.29 + 0.5 * a );
    float s2    = s * s;
    float VoL   = 2 * angularInfo.VoH * angularInfo.VoH - 1; // double angle identity
    float Cosri = VoL - angularInfo.NoV * angularInfo.NoL;
    float C1    = 1 - 0.5 * s2 / (s2 + 0.33);
    float C2    = 0.45 * s2 / (s2 + 0.09) * Cosri * (Cosri >= 0 ? rcp(max(angularInfo.NoL, angularInfo.NoV + 0.0001f)) : 1);

    return albedo * INV_PI * (C1 + C2) * (1 + roughness * 0.5) * angularInfo.NoL;
  }
}

// Normalized Disney diffuse function taken from Frostbite's PBR course notes (page 10):
// https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/course-notes-moving-frostbite-to-pbr-v32.pdf
public struct DiffuseFrostbite : IBRDF
{
  float3 albedo;
  float  roughness;

  __init(float3 albedo, float linearRoughness)
  {
    this.albedo    = albedo;
    this.roughness = linearRoughness;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    const float3 f0 = float3(1.0, 1.0, 1.0);

    float energyBias   = lerp(0.0, 0.5, roughness);
    float energyFactor = lerp(1.0, 1.0 / 1.51, roughness);
    float fd90         = energyBias + 2.0 * angularInfo.VoH * angularInfo.VoH * roughness;
    float lightScatter = F_Schlick(f0, fd90, angularInfo.NoL).r;
    float viewScatter  = F_Schlick(f0, fd90, angularInfo.NoV).r;

    return albedo * lightScatter * viewScatter * energyFactor * INV_PI * angularInfo.NoL;
  }
}

// Extended Lambertian + Fresnel diffuse model from Respawn's GDC2017 slides(page 113):
// https://twvideo01.ubm-us.net/o1/vault/gdc2017/Presentations/Hammon_Earl_PBR_Diffuse_Lighting.pdf
public struct DiffuseTitanFall : IBRDF
{
  float3 albedo;
  float  roughness;

  __init(float3 albedo, float roughnessA)
  {
    this.albedo    = albedo;
    this.roughness = roughnessA;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    const float3 f0 = float3(1.0, 1.0, 1.0);

    // Squared length of L + V <=> | L + V |^2
    float LplusV_LenSq = 2.0 + 2.0 * saturate(dot(angularInfo.V, angularInfo.L));
    float NdotH        = (angularInfo.NoL + angularInfo.NoV) * rsqrt(LplusV_LenSq);

    float facing = LplusV_LenSq * 0.25;

    // empirical formula for rough surfaces
    float rough = facing * (0.9 - 0.4 * facing) * ((0.5 + NdotH) / NdotH);

    // used double Fresnel as the smooth term,
    // though it can be replaced with other diffuse function as well like pure lambertian
    float smooth = 1.05 * F_Schlick(f0, 0.0, angularInfo.NoL).x * F_Schlick(f0, 0.0, angularInfo.NoV).x;
    float single = INV_PI * lerp(smooth, rough, roughness);

    // coarse approximation of multiple scattering
    float multi = 0.1159 * roughness;

    return albedo * (single + albedo * multi) * angularInfo.NoL;
  }
}
