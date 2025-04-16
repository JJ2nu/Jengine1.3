#include "PBRShared.fxh"

// �ν��Ͻ� ������ ����ü �߰�
struct InstanceData
{
    float4x4 World;
    float4 AnimParams; // x:width tiles, y:height tiles, z:current frame,
    float4 Color;
};

StructuredBuffer<InstanceData> InstanceBuffer : register(t0); // �ν��Ͻ� ������ ����





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
    
    // �ν��Ͻ� ������ ��������
    InstanceData instance = InstanceBuffer[vin.InstanceID];
    
    // ���� ��ȯ (�ν��Ͻ���)
    float4x4 instanceWorld = instance.World;
    vout.Pos = mul(float4(vin.Pos, 1.0), instanceWorld);
    
    // ��/�������� ��ȯ
    vout.WorldPos = vout.Pos;
    vout.Pos = mul(vout.Pos, View);
    vout.Pos = mul(vout.Pos, Projection);
    vout.Norm = normalize(mul(normalize(vin.Norm), (float3x3) instanceWorld));
    vout.Tangent = normalize(mul(normalize(vin.Tangent), (float3x3) instanceWorld));
    vout.Binormal = normalize(mul(normalize(vin.Binormal), (float3x3) instanceWorld));

    
    
    vout.Tex = UVAnimate(vin.Tex, instance.AnimParams);
    vout.ColorBlend = instance.Color;
    return vout;
}
