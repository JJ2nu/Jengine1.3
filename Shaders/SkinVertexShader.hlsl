#include "SkinShared.fxh"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    float4 pos = input.Pos;

    float4x4 matWorld = World; 
#ifdef Skinning
    matWorld = mul(input.BlendWeight.x, MatrixPalleteArray[input.BlendIndices.x]);
    matWorld += mul(input.BlendWeight.y, MatrixPalleteArray[input.BlendIndices.y]);
    matWorld += mul(input.BlendWeight.z, MatrixPalleteArray[input.BlendIndices.z]);
    matWorld += mul(input.BlendWeight.w, MatrixPalleteArray[input.BlendIndices.w]);
#endif
    pos = mul(pos, matWorld);
    output.WorldPos = pos.xyz;

    pos = mul(pos, View);
    pos = mul(pos, Projection);
     
    output.Pos = pos;
    
    output.Norm = normalize(mul(normalize(input.Norm), (float3x3) matWorld));
    output.Tangent = normalize(mul(normalize(input.Tangent), (float3x3) matWorld));
    output.Binormal = input.Binormal;
  //  if (length(input.Binormal) >= 1.0f)
    output.Binormal = normalize(mul(normalize(input.Binormal), (float3x3) matWorld));

    output.PosShadow = mul(output.WorldPos, ShadowView);
    output.PosShadow = mul(output.PosShadow, ShadowProjection);


	output.Tex = input.Tex;

    return output;
}