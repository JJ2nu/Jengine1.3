#include "SkyBoxShared.fxh"

float4 main(PS_INPUT pin) : SV_Target
{
    float3 envVector = normalize(pin.Pos);
    return envTexture.SampleLevel(defaultSampler, envVector, 0);
}