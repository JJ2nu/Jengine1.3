#include "Shared.fxh"

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

    float3 NormalizedLightDir = normalize((float3) light[0].lightDir);

    float4 finalDiffuse = saturate(dot((float3) -NormalizedLightDir, inputNorm)) * mat.diffuse * light[0].diffuse;
    if (0 < length(diffuseTexture))
    {
        finalDiffuse *= diffuseTexture;

    }

    float3 ViewVector = normalize((float3) EyePos - (float3) input.WorldPos);
    float3 HalfVector = normalize(-NormalizedLightDir + ViewVector);
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

    finalColor.rgb = pow(finalColor.rgb, 1 / 2.2);


    if (flag1)
        return finalColor; //    +IBL;
    else
        return txDiffuse.Sample(samLinear, input.Tex);
}

