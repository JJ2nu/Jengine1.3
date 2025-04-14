// BloomCombine.hlsl
#include "PBRShared.fxh"
Texture2D originalTexture : register(t0); // 원본 텍스처
Texture2D bloomTexture : register(t1); // Bloom 텍스처
SamplerState samplerState : register(s0); // 샘플러 상태

cbuffer BloomParams : register(b0)
{
    float bloomthreshold;
    float bloomintensity;
    float2 blurDirection; // 블러 방향 (수평 또는 수직)
    int blurRadius;
    float dummy;
    float dummy2;
    float dummy3;
};


float4 PS_Combine(QUAD_PS_INPUT input) : SV_Target
{
    float3 originalColor = originalTexture.Sample(samplerState, input.uv).rgb;
    float3 bloomColor = bloomTexture.Sample(samplerState, input.uv).rgb;
	float4 output = float4(originalColor + bloomColor * bloomintensity, 1); // 합성
    //output.rgb = ToneMap(output.rgb,5.f);
    return output;
}
