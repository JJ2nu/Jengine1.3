#include "SkyBoxShared.fxh"


PS_INPUT main(VS_INPUT vin)
{
    PS_INPUT vout = (PS_INPUT) 0;
    vout.WorldPos = mul(vin.Pos, World);
    vout.Pos = mul(vin.Pos, World);
    return vout;
}