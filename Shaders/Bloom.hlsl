// Bloom.hlsl
Texture2D inputTexture : register(t0); // 원본 텍스처
Texture2D bloomTexture : register(t1); // Bloom 텍스처 (Combine 단계에서 사용)
SamplerState samplerState : register(s0); // 샘플러 상태

cbuffer BloomParams : register(b0)
{
    float threshold; // 밝기 임계값
    float bloomIntensity; // Bloom 강도
    float2 blurDirection; // 블러 방향 (수평 또는 수직)
};

float weights[5] = { 0.227027f, 0.316216f, 0.070270f, 0.002216f, 0.000216f }; // Gaussian 커널

// 1. Thresholding 단계: 밝은 영역만 추출
float4 ExtractBrightAreas(float2 uv)
{
    float3 color = inputTexture.Sample(samplerState, uv).rgb;
    float intensity = dot(color, float3(0.3f, 0.59f, 0.11f)); // 밝기 계산 (YUV 변환)
    if (intensity > threshold)
    {
        return float4(color, 1.0); // 밝은 영역 유지
    }
    else
    {
        return float4(0.0, 0.0, 0.0, 1.0); // 어두운 영역 제거
    }
}

// 2. Gaussian Blur 단계: 부드러운 흐림 효과 적용
float4 ApplyGaussianBlur(float2 uv)
{
    float4 result = float4(0, 0, 0, 1);
    for (int i = -2; i <= 2; i++)
    {
        result += inputTexture.Sample(samplerState, uv + blurDirection * i) * weights[i + 2];
    }
    return result;
}

// 3. Combine 단계: 원본 이미지와 Bloom 이미지를 합성
float4 CombineBloom(float2 uv)
{
    float4 originalColor = inputTexture.Sample(samplerState, uv);
    float4 bloomColor = bloomTexture.Sample(samplerState, uv);
    return originalColor + bloomColor * bloomIntensity;
}

// 메인 셰이더 함수들
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
