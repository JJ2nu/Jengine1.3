

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 EyePos;

}
cbuffer ShadowTransform : register(b2)
{
    matrix ShadowView;
    matrix ShadowProjection;
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
    float4 ShadowPos : TEXCOORD1;

};

PS_INPUT main (VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    float4 pos = input.Pos;


	float4x4 matWorld = World;

    pos = mul(pos, matWorld);
    pos = mul(pos, ShadowView);
    pos = mul(pos, ShadowProjection);
    output.Pos = pos;
    return output;
}