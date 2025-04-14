
// BloomBlur.hlsl
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


float4 PS_Blur(QUAD_PS_INPUT input) : SV_Target
{
    //float4 result = float4(0, 0, 0, 1);
    //for (int i = -2; i <= 2; i++)
    //{
    //    result += inputTexture.Sample(samplerState, input.uv + blurDirection * i) * weights[i + 2];
    //}
    //return result;
    float4 accumulatedValue = float4(0.0, 0.0, 0.0, 0.0);
    float2 texelSize = blurDirection; // �ؽ�ó ��ǥ���� �̵� ũ��

    // ����þ� �� ����
    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        float coefficient = coefficients[abs(i)];
        float2 offset = texelSize * i;
        accumulatedValue += coefficient * inputTexture.Sample(samplerState, input.uv + offset);
    }

    //return accumulatedValue;
    return saturate(accumulatedValue);





    
}
