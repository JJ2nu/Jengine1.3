#include "PBRShared.fxh"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT vin)
{
    PS_INPUT vout;

	float4x4 matWorld = World;
#ifdef Skinning
    matWorld = mul(vin.BlendWeight.x, MatrixPalleteArray[vin.BlendIndices.x]);
    matWorld += mul(vin.BlendWeight.y, MatrixPalleteArray[vin.BlendIndices.y]);
    matWorld += mul(vin.BlendWeight.z, MatrixPalleteArray[vin.BlendIndices.z]);
    matWorld += mul(vin.BlendWeight.w, MatrixPalleteArray[vin.BlendIndices.w]);
#endif
    vout.Pos = mul(vin.Pos, matWorld);
	vout.WorldPos = vout.Pos;
    vout.Pos = mul(vout.Pos, View);
    vout.Pos = mul(vout.Pos, Projection);
    vout.Tex = float2(vin.Tex.x, vin.Tex.y);

	// Pass tangent space basis vectors (for normal mapping).
    vin.Tangent = mul(vin.Tangent, (float3x3) World);
    vout.Tangent = normalize(vin.Tangent);
    vin.Binormal = mul(vin.Binormal, (float3x3) World);
    vout.Binormal = normalize(vin.Binormal);
    vin.Norm = mul(vin.Norm, (float3x3) World);
    vout.Norm = normalize(vin.Norm);
    //vout.tangentBasis = float3x3(vin.Tangent, vin.Binormal, vin.Norm);
    vout.PosShadow = mul(vout.WorldPos, ShadowView);
    vout.PosShadow = mul(vout.PosShadow, ShadowProjection);
    return vout;
}
