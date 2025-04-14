// BloomExtract.hlsl
#include "PBRShared.fxh"

Texture2D inputTexture : register(t0); // 입력 텍스처
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

float4 PS_Extract(QUAD_PS_INPUT input) : SV_Target
{
    float3 color = inputTexture.Sample(samplerState, input.uv).rgb;
    float intensity = dot(color, float3(0.3f, 0.59f, 0.11f)); // 밝기 계산 (YUV 변환)
    if (intensity > bloomthreshold)
    {
        return float4(color, 1.f); // 밝은 영역 유지
    }
    else
    {
        return float4(0.0, 0.0, 0.0, 1.0); // 어두운 영역 제거
    }
}
