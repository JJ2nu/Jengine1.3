#include "PBRShared.fxh"

// 인스턴스 데이터 구조체 추가
struct InstanceData
{
    float4x4 World;
    float4 AnimParams; // x:width tiles, y:height tiles, z:current frame,
    float4 Color;
};

StructuredBuffer<float4x4> InstanceBuffer : register(t0); // 인스턴스 데이터 버퍼
StructuredBuffer<float4> AnimBuffer : register(t1); // 인스턴스 데이터 버퍼
StructuredBuffer<float4> ColorBuffer : register(t2); // 인스턴스 데이터 버퍼
StructuredBuffer<float4> age : register(t3); // 인스턴스 데이터 버퍼

float2 UVAnimate(float2 uv, float4 AnimParams)
{
	
    float2 frameuv = uv;
    int2 index = int2(AnimParams.z % (uint) AnimParams.x, AnimParams.z / (uint) AnimParams.x);
    float2 offset = float2(1.f / AnimParams.x, 1.f / AnimParams.y);
    float2 curframe = float2(index.x * offset.x, index.y * offset.y);

    frameuv = frameuv * offset + curframe;

    return frameuv;
}
PARTICLE_PS_INPUT main(PARTICLE_VS_INPUT vin)
{
    PARTICLE_PS_INPUT vout;
    
    // 인스턴스 데이터 가져오기
    float4x4 instanceWorld = InstanceBuffer[vin.InstanceID];
    // 월드 변환 (인스턴스별)
    vout.Pos = mul(float4(vin.Pos), instanceWorld);
    
    // 뷰/프로젝션 변환
    vout.WorldPos = vout.Pos;
    vout.Pos = mul(vout.Pos, View);
    vout.Pos = mul(vout.Pos, Projection);
    vout.Norm = normalize(mul(normalize(vin.Norm), (float3x3) instanceWorld));
    vout.Tangent = normalize(mul(normalize(vin.Tangent), (float3x3) instanceWorld));
    vout.Binormal = normalize(mul(normalize(vin.Binormal), (float3x3) instanceWorld));
    vout.Tex = UVAnimate(vin.Tex, AnimBuffer[vin.InstanceID]);
    vout.ColorBlend = ColorBuffer[vin.InstanceID];
    vout.PosShadow = age[vin.InstanceID];

    return vout;
}
