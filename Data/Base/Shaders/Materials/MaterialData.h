#pragma once

#include <Shaders/Common/Common.h>

struct ezMaterialData
{
  float3 worldPosition;
  float3 worldNormal;
  float3 vertexNormal;

  float3 diffuseColor;
  float3 specularColor;
  float3 emissiveColor;
  float4 refractionColor;

  float roughness;
  float perceptualRoughness;
  float metalness;
  float occlusion;
  float cavity;
  float opacity;

  float clearcoat;
  float clearcoatRoughness;
  float3 clearcoatNormal;

  float anisotropic;
  float anisotropicRotation;

  float sheen;
  float sheenTintFactor;

  float2 velocity;

  float3 subsurfaceColor;
  float subsurfaceScatterPower;
  float subsurfaceShadowFalloff;
};
