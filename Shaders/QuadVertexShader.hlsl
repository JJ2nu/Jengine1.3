#include "PBRShared.fxh"

QUAD_PS_INPUT main(QUAD_VS_INPUT input)
{
    QUAD_PS_INPUT output = (QUAD_PS_INPUT) 0;
    input.position.w = 1.0f;
    output.position = input.position;
    output.uv = input.uv;
    return output;
}