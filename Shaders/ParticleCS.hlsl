#include "PBRShared.fxh"
StructuredBuffer<float> InputBuffer : register(t0);
RWStructuredBuffer<float> OutputBuffer : register(u0);


[numthreads(32, 1, 1)]
void main(
    uint3 groupID : SV_GroupID,
    uint3 threadID : SV_DispatchThreadID)
{
    uint idx = threadID.x;
    OutputBuffer[idx] = InputBuffer[idx] * 2.0f;
}