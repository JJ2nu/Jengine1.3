#include "PBRShared.fxh"



float2 UVAnimate(float2 uv)
{
	
    float2 frameuv = uv;
    int2 index = int2(frameCnt.z % (uint) frameCnt.x, frameCnt.z / (uint) frameCnt.x);
    float2 offset = float2(1.f / frameCnt.x, 1.f / frameCnt.y);
    float2 curframe = float2(index.x * offset.x, index.y * offset.y);

    frameuv = frameuv * offset + curframe;

    return frameuv;
}


PS_INPUT main(PARTICLE_VS_INPUT vin)
{
    PS_INPUT vout;

    vout.Pos = mul(vin.Pos, World);
    vout.WorldPos = vout.Pos;
    vout.Pos = mul(vout.Pos, View);
    vout.Pos = mul(vout.Pos, Projection);

    vout.Norm = normalize(mul(normalize(vin.Norm), (float3x3) World));
    vout.Tangent = normalize(mul(normalize(vin.Tangent), (float3x3) World));
    vout.Binormal = normalize(mul(normalize(vin.Binormal), (float3x3) World));

    
    
    
    vout.Tex = UVAnimate(vin.Tex);
    return vout;
}
