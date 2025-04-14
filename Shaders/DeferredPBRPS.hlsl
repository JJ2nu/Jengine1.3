#include "PBRShared.fxh"

DeferredOut main(PS_INPUT input) : SV_TARGET
{
    DeferredOut output = (DeferredOut) 0;
    ////skybox
    //if (mat.emissive.x == 1.0f)
    //{
    //    float4 projpos = mul(input.WorldPos, View);
    //    projpos = mul(projpos, Projection);


    //    float3 envVector = normalize(projpos.xyzw);
    //    output.Albedo = txEnv.SampleLevel(samLinear, envVector, 0);
    //    output.Material.b = 0.0f;
    //    return output;
    //}


        float depth = input.Pos.z;
    float linearDepth = (nearFar.y * nearFar.x) / (nearFar.y - depth * (nearFar.y - nearFar.x));

	output.Albedo = txDiffuse.Sample(samLinear, input.Tex);

	output.ShadowPosition = input.PosShadow / input.PosShadow.w;

	float metalness = txMetalness.Sample(samLinear, input.Tex).r;
	float roughness = txRoughness.Sample(samLinear, input.Tex).r;
    output.Material.r = metalness;
    output.Material.g = roughness;
    output.Material.b = depth;
    output.Material.a = 1.f;


    float3 normalizedNormal = input.Norm;
    float3 normalizedTangent = normalize(input.Tangent);
    float3 normalizedBiTangent = normalize(input.Binormal);
    float3x3 TangentSpace = float3x3(normalizedTangent, normalizedBiTangent, normalizedNormal);
    float4 normalColor = txNormal.Sample(samLinear, input.Tex);
    float3 normColorSRGB = (normalColor.rgb * 2.f) - 1.f;
    if (0.f < length(normalColor))
    {
        normalizedNormal = mul(normColorSRGB, TangentSpace);
    }
    normalizedNormal = normalize(normalizedNormal);
    output.Normal = float4(normalizedNormal, 1.f);

    output.Emissive = txEmissive.Sample(samLinear, input.Tex);

    return output;
}