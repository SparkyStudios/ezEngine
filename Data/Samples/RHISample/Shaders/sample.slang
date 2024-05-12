struct VS_INPUT
{
  float3 pos : POSITION;
  float3 nrm : Normal;
  float3 tnt : Tangent;
  float3 btt : BiTangent;
  float2 uv0 : TexCoord0;
}

struct VS_OUTPUT
{
  float4 pos : SV_POSITION;
  float2 uv0 : TEXCOORD0;
}

[shader("vertex")]
VS_OUTPUT VSMain(VS_INPUT input)
{
  VS_OUTPUT output;
  output.pos = float4(input.pos, 1.0f);
  output.uv0 = input.uv0;
  return output;
}

SamplerState linearSampler;

cbuffer settings
{
  float4 color;
}

Texture2D<float4> tex;

[shader("pixel")]
float4 PSMain(VS_OUTPUT input)
  : SV_Target
{
  return tex.Sample(linearSampler, input.uv0) * color;
}
