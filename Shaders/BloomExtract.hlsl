// BloomExtract.hlsl
#include "PBRShared.fxh"

Texture2D inputTexture : register(t0); // �Է� �ؽ�ó
SamplerState samplerState : register(s0); // ���÷� ����

cbuffer BloomParams : register(b0)
{
    float bloomthreshold;
    float bloomintensity;
    float2 blurDirection; // �� ���� (���� �Ǵ� ����)
    int blurRadius;
    float dummy;
    float dummy2;
    float dummy3;
};

float4 PS_Extract(QUAD_PS_INPUT input) : SV_Target
{
    float3 color = inputTexture.Sample(samplerState, input.uv).rgb;
    float intensity = dot(color, float3(0.3f, 0.59f, 0.11f)); // ��� ��� (YUV ��ȯ)
    if (intensity > bloomthreshold)
    {
        return float4(color, 1.f); // ���� ���� ����
    }
    else
    {
        return float4(0.0, 0.0, 0.0, 1.0); // ��ο� ���� ����
    }
}
