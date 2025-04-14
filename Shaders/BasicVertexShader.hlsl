#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.WorldPos = output.Pos;

    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);


    output.Norm = normalize(mul(normalize(input.Norm), (float3x3) World));
    output.Tangent = normalize(mul(normalize(input.Tangent), (float3x3) World));
    output.Binormal = input.Binormal;
  //  if (length(input.Binormal) >= 1.0f)
    output.Binormal = normalize(mul(normalize(input.Binormal), (float3x3) World));

	output.Tex = input.Tex;

    output.PosShadow = mul(float4(output.WorldPos, 1.0f), ShadowView);
    output.PosShadow = mul(output.PosShadow, ShadowProjection);


    return output;
}