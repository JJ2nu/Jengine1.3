#include "PBRShared.fxh"
Texture2D BillBoardTexture : register(t17);
Texture2D opacityTexture : register(t18);
Texture2D normalTexture : register(t19);
float4 main(PARTICLE_PS_INPUT input) : SV_Target
{
    float4 output = BillBoardTexture.Sample(samLinear, input.Tex);
    output.rgb = output.rgb * input.ColorBlend.rgb;
    float4 alpha = opacityTexture.Sample(samLinear, input.Tex);

    float4 normal = normalTexture.Sample(samLinear, input.Tex);
    float3 vNormal = normalize(input.Norm);

    float3 vTangent = normalize(input.Tangent);
    float3 vBinormal = normalize(input.Binormal);
    float3x3 vTBNWorld = float3x3(vTangent, vBinormal, vNormal);
	float3 inputnorm = float3(0,0,-1);
    if (0 < length(normal))
    {
        inputnorm = normalize(mul(normal.rgb * 2.0f - 1.0f, vTBNWorld));
        //inputnorm = normalize(normal.rgb * 2.0f - 1.0f);
    }

    float3 NormalizedLightDir = normalize((float3) light[0].lightDir);
    float4 finalDiffuse = saturate(dot((float3) -NormalizedLightDir, inputnorm));
    float3 ViewVector = normalize((float3) EyePos - (float3) input.WorldPos);
    float3 HalfVector = normalize(-NormalizedLightDir + ViewVector);
    float fHDotN = max(0.0f, dot(HalfVector, inputnorm));
    if (length(normal)>0)
    {
	    
        output += finalDiffuse;
        //output += fHDotN;
    }

    if (length(alpha) > 0)
        output.a = alpha.r;
    output.a *= input.ColorBlend.w;
    return output;
}


