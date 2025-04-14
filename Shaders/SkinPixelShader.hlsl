#include "SkinShared.fxh"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    float4 finalColor = 0;
    float3 vNormal = normalize(input.Norm);

    float3 vTangent = normalize(input.Tangent);
    float3 vBinormal = normalize(input.Binormal);
    float3x3 vTBNWorld = float3x3(vTangent, vBinormal, vNormal);
    float3 inputNorm = vNormal;


    float4 diffuseTexture = txDiffuse.Sample(samLinear, input.Tex);
    diffuseTexture.rgb = pow(diffuseTexture.rgb, 2.2);
    float4 normalTexture = txNormal.Sample(samLinear, input.Tex);
    float4 specularTexture = txSpecular.Sample(samLinear, input.Tex);
    float4 emissiveTexture = txEmissive.Sample(samLinear, input.Tex);
    float4 opacityTexture = txOpacity.Sample(samLinear, input.Tex);



    if (0 < length(normalTexture))
    {
        inputNorm = normalize(mul(normalTexture.rgb * 2.0f - 1.0f, vTBNWorld));
    }

    float3 NormalizedlightDir = normalize((float3) light[0].lightDir);

    float4 finalDiffuse = saturate(dot((float3) -NormalizedlightDir, inputNorm)) * mat.diffuse * light[0].diffuse;
    if (0 < length(diffuseTexture))
    {
        finalDiffuse *= diffuseTexture;

    }

    float3 ViewVector = normalize((float3) EyePos - (float3) input.WorldPos);
    float3 HalfVector = normalize(-NormalizedlightDir + ViewVector);
    float fHDotN = max(0.0f, dot(HalfVector, inputNorm));

    float4 finalSpecular = pow(fHDotN, mat.specularExponent.x) * light[0].specular * mat.specular;
    if (0 < length((specularTexture)))
    {
        finalSpecular *= specularTexture;
    }
					

    float3 Reflection = reflect(ViewVector, inputNorm);
    float4 IBL = txEnv.Sample(samLinear, Reflection);
    float4 finalAmbient = light[0].ambient * mat.ambient;

    finalColor += finalAmbient;
    finalColor += finalDiffuse;
    finalColor += finalSpecular;

    if (0 < length((emissiveTexture)))
        finalColor += emissiveTexture;

    if (0 < length((opacityTexture)))
        finalColor.a = opacityTexture.w;
    else if (diffuseTexture.a != 1)
        finalColor.a = diffuseTexture.a;
    else
        finalColor.a = 1;


    float currentShadowDepth = input.PosShadow.z / input.PosShadow.w;
    float2 shadowUV = input.PosShadow.xy / input.PosShadow.w;
    shadowUV.y = -shadowUV.y;
    shadowUV = shadowUV * 0.5 + 0.5;
    float shadowFactor = 1.0f;
    if (shadowUV.x >= 0.0 && shadowUV.x <= 1.0 && shadowUV.y >= 0.0 && shadowUV.y <= 1.0)
    {
        //float sampelShadowDepth = txShadow.Sample(samLinear, shadowUV).r;
        //if (currentShadowDepth > sampelShadowDepth + 0.001)
        //{
        //    shadowFactor = 0.0f;
        //}
        float2 offset[9] =
        {
            float2(-1, -1), float2(0, -1), float2(1, -1),
			float2(-1, 0), float2(0, 0), float2(1, 0),
			float2(-1, 1), float2(0, 1), float2(1, 1)
        };
        float texelSize = 1.0 / 8192;
        shadowFactor = 0.0f;
        [unroll]
        for (int i = 0; i < 9; i++)
        {
            float2 sampleUV = shadowUV + offset[i] * texelSize;
            float temp = txShadow.SampleCmpLevelZero(samplerComparison, sampleUV, currentShadowDepth - 0.001f);
            
            shadowFactor += temp;
        }
        shadowFactor = shadowFactor / 9.0f;

    }



    finalColor.rgb = pow(finalColor.rgb, 1 / 2.2);


    if (flag1)
        return finalColor * shadowFactor;
    else
        return txDiffuse.Sample(samLinear, input.Tex);
}

