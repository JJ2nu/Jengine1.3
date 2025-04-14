// Bloom.hlsl
Texture2D inputTexture : register(t0); // ���� �ؽ�ó
Texture2D bloomTexture : register(t1); // Bloom �ؽ�ó (Combine �ܰ迡�� ���)
SamplerState samplerState : register(s0); // ���÷� ����

cbuffer BloomParams : register(b0)
{
    float threshold; // ��� �Ӱ谪
    float bloomIntensity; // Bloom ����
    float2 blurDirection; // �� ���� (���� �Ǵ� ����)
};

float weights[5] = { 0.227027f, 0.316216f, 0.070270f, 0.002216f, 0.000216f }; // Gaussian Ŀ��

// 1. Thresholding �ܰ�: ���� ������ ����
float4 ExtractBrightAreas(float2 uv)
{
    float3 color = inputTexture.Sample(samplerState, uv).rgb;
    float intensity = dot(color, float3(0.3f, 0.59f, 0.11f)); // ��� ��� (YUV ��ȯ)
    if (intensity > threshold)
    {
        return float4(color, 1.0); // ���� ���� ����
    }
    else
    {
        return float4(0.0, 0.0, 0.0, 1.0); // ��ο� ���� ����
    }
}

// 2. Gaussian Blur �ܰ�: �ε巯�� �帲 ȿ�� ����
float4 ApplyGaussianBlur(float2 uv)
{
    float4 result = float4(0, 0, 0, 1);
    for (int i = -2; i <= 2; i++)
    {
        result += inputTexture.Sample(samplerState, uv + blurDirection * i) * weights[i + 2];
    }
    return result;
}

// 3. Combine �ܰ�: ���� �̹����� Bloom �̹����� �ռ�
float4 CombineBloom(float2 uv)
{
    float4 originalColor = inputTexture.Sample(samplerState, uv);
    float4 bloomColor = bloomTexture.Sample(samplerState, uv);
    return originalColor + bloomColor * bloomIntensity;
}

// ���� ���̴� �Լ���
float4 PS_Extract(float2 uv : TEXCOORD) : SV_Target
{
    return ExtractBrightAreas(uv);
}

float4 PS_Blur(float2 uv : TEXCOORD) : SV_Target
{
    return ApplyGaussianBlur(uv);
}

float4 PS_Combine(float2 uv : TEXCOORD) : SV_Target
{
    return CombineBloom(uv);
}
