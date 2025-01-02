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

public struct SpecularIsotropic : IBRDF
{
  private float  roughness;
  private float3 specular;
  private float  metalness;

  __init(Surface surface)
  {
    this.roughness = surface.Roughness;
    this.specular  = surface.Specular;
    this.metalness = surface.Metalness;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    float a  = roughness;
    float a2 = a * a;

    float  V = V_SmithJointApprox(a, angularInfo.NoV, angularInfo.NoL);
    float  D = D_GGX(a2, angularInfo.NoH);
    float3 F = F_Schlick(specular, angularInfo.VoH);

    surface.kD *= ComputeDiffuseEnergy(F, metalness);
    surface.kS *= F;

    return D * V * F;
  }
}

public struct SpecularAnisotropic : IBRDF
{
  float  roughness;
  float3 specular;
  float  metalness;
  float  anisotropic;
  float  anisotropicRotation;

  __init(float anisotropic, float anisotropicRotation, float roughnessA, float3 specular, float metalness)
  {
    this.roughness           = roughnessA;
    this.specular            = specular;
    this.metalness           = metalness;
    this.anisotropic         = anisotropic;
    this.anisotropicRotation = anisotropicRotation;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    // Construct TBN from the normal
    float3 t, b;
    FindBestAxisVectors(angularInfo.N, t, b);
    float3x3 TBN = float3x3(t, b, angularInfo.N);

    // Rotate tangent and bitagent
    float  rotation  = max(anisotropicRotation * PI2, FLT_MIN);          // convert material property to a full rotation
    float2 direction = float2(cos(rotation), sin(rotation));             // convert rotation to direction
    t                = normalize(mul(float3(direction, 0.0f), TBN).xyz); // compute direction derived tangent
    b                = normalize(cross(angularInfo.N, t));               // re-compute bitangent

    float alpha_ggx = roughness;
    float aspect    = sqrt(1.0 - anisotropic * 0.9);
    float ax        = alpha_ggx / aspect;
    float ay        = alpha_ggx * aspect;
    float XdotH     = dot(t, angularInfo.H);
    float YdotH     = dot(b, angularInfo.H);

    // specular anisotropic BRDF
    float  D   = D_GGX_Anisotropic(angularInfo.NoH, ax, ay, XdotH, YdotH);
    float  V   = V_GGX_anisotropic_2cos(angularInfo.NoV, ax, ay, XdotH, YdotH) * V_GGX_anisotropic_2cos(angularInfo.NoV, ax, ay, XdotH, YdotH);
    float  f90 = saturate(dot(specular, 50.0 * GetLuminance(specular)));
    float3 F   = F_Schlick(specular, f90, angularInfo.LoH);

    surface.kD *= ComputeDiffuseEnergy(F, metalness);
    surface.kS *= F;

    return D * V * F;
  }
}

public struct SpecularClearCoat : IBRDF
{
  float clearcoat;
  float roughness;
  float metalness;

  __init(float clearcoat, float clearcoatRoughness, float metalness)
  {
    this.clearcoat = clearcoat;
    this.roughness = clearcoatRoughness;
    this.metalness = metalness;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    float a2 = max(roughness * roughness, 0.0005);
    float a4 = a2 * a2;

    float  D = D_GGX(a4, angularInfo.NoH);
    float  V = V_Kelemen(angularInfo.VoH);
    float3 F = F_Schlick(0.04, 1.0, angularInfo.VoH) * clearcoat;

    surface.kD *= ComputeDiffuseEnergy(F, metalness);
    surface.kS *= F;

    return D * V * F;
  }
}

public struct SpecularSheen : IBRDF
{
  float  factor;
  float  roughness;
  float  sheen;
  float3 specular;
  float  metalness;

  __init(float sheen, float sheenTintFactor, float roughness, float metalness, float3 specular)
  {
    this.sheen     = sheen;
    this.factor    = sheenTintFactor;
    this.roughness = roughness;
    this.metalness = metalness;
    this.specular  = specular;
  }

  public float3 evaluate(inout Surface surface, AngularInfo angularInfo)
  {
    // Mix between white and using base color for sheen reflection
    float  tint = factor * factor;
    float3 f0   = lerp(1.0f, specular, tint);

    float  D = D_Charlie(roughness, angularInfo.NoH);
    float  V = V_Neubelt(angularInfo.NoV, angularInfo.NoL);
    float3 F = f0 * sheen;

    surface.kD *= ComputeDiffuseEnergy(F, metalness);
    surface.kS *= F;

    return D * V * F;
  }
}
