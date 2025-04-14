cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 EyePos;

}
struct Light
{
    float4 lightDir;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    float4 lightSource;
    float4 radiance;

};
struct Material
{
    float4 metalness;
    float4 roughness;
    float4 specular;
    float4 emissive;
    float4 specularExponent;

};
cbuffer LightBuffer : register(b1)
{
    Light light[4];
    Material mat;
    bool flag1 = false;
    float3 pad;
    bool flag2 = false;
    float3 pad2;
}
struct VS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Tex : TEXCOORD0;
    int4 BlendIndices : BLENDINDICES;
    float4 BlendWeight : BLENDWEIGHTS;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 PosShadow : TEXCOORD1;
};

TextureCube envTexture : register(t5);
SamplerState defaultSampler : register(s0);
