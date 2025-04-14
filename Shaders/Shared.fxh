Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txSpecular : register(t2);
Texture2D txEmissive : register(t3);
Texture2D txOpacity : register(t4);
TextureCube txEnv : register(t5);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
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
    float4 lightRadiance;
};
struct Material
{
    float4 ambient;
    float4 diffuse;
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

cbuffer ShadowTransform : register(b2)
{
    matrix ShadowView;
    matrix ShadowProjection;
}



//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 WorldPos : POSITION;
    float2 Tex : TEXCOORD0;
    float4 PosShadow : TEXCOORD1;

};












