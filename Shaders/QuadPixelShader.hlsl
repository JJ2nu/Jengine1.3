#include "PBRShared.fxh"

uint querySpecularuvtureLevels()
{
    uint width, height, levels;
    txSpecular.GetDimensions(0, width, height, levels);
    return levels;
}

float4 main(QUAD_PS_INPUT pin) : SV_Target
{
    float3 directLighting = float3(0.f, 0.f, 0.f);

    float roughness = deferredMaterial.Sample(samLinear, pin.uv).g * mat.roughness.x;
    float roughnessSqaure = pow(max(0.01, roughness), 4.0f);
    float metalness = deferredMaterial.Sample(samLinear, pin.uv).r * mat.metalness.x;
    float3 albedo = deferredAlbedo.Sample(samLinear, pin.uv).rgb;
    if (length(albedo))
        albedo = pow(albedo.rgb, 2.2);
    else
        albedo = mat.specular.rgb;

    float3 N = deferredNormal.Sample(samLinear, pin.uv).rgb;

    float depthColor = deferredMaterial.Sample(samLinear, pin.uv).b;
    if (depthColor == 1.f)
        return float4(albedo, 1.f);


    float3 ndcPosition = float3(pin.uv * 2.0f - 1.0f, depthColor.r);
    ndcPosition.y = -ndcPosition.y;
    float4 clipPosition = float4(ndcPosition, 1.0f);
    float4 worldPosition = mul(clipPosition, InverseProjectionMatrix);
    worldPosition = mul(worldPosition, InverseViewMatrix);
    worldPosition /= worldPosition.w; // Homogeneous divide

    float3 WorldPos = worldPosition.xyz;



    float3 ViewVector = normalize(EyePos.xyz - WorldPos);
    float cosVN = max(0.0, dot(N, ViewVector));
    float3 LightReflection = 2.0f * cosVN * N - ViewVector;


    float3 F0 = lerp(Fdielectric, albedo, metalness);

    for (uint i = 0; i < 1; ++i)
    {
        float3 LightIn = normalize(-light[i].lightDir.xyz);
        float3 H = normalize(LightIn + ViewVector);
        float3 LightRadiance = light[i].radiance.xyz;
        
        float NormalDistribution = roughnessSqaure / (PI * (pow((dot(N, H) * dot(N, H) * (roughnessSqaure - 1) + 1), 2)));

        float3 FresnelSchlickReflection = F0 + (1.f - F0) * pow((1 - max(0.0f, dot(H, ViewVector))), 5);


        float cosLightIn = max(0.0, dot(N, LightIn));
        float cosLH = max(0.0, dot(N, H));
        float k = pow(roughness + 1.0, 2.0) / 8.0;
        float GeometricAttenuation = (cosLightIn / (cosLightIn * (1.0 - k) + k)) * (cosVN / (cosVN * (1.0 - k) + k));


        float3 CookTorrenceSpecularBRDF = (FresnelSchlickReflection * NormalDistribution * GeometricAttenuation) / max(Epsilon, (4 * cosLightIn * cosVN));


        float3 kd = lerp(float3(1, 1, 1) - FresnelSchlickReflection, float3(0, 0, 0), metalness);
        float3 DiffuseBRDF = kd * albedo / PI;

        directLighting += (DiffuseBRDF + CookTorrenceSpecularBRDF) * LightRadiance * cosLightIn;

    }



	// Ambient lighting (IBL).
    float3 ambientLighting;

    float3 irradiance = txIrridiance.Sample(samLinear, N).rgb;
    float3 F = F0 + (1.0 - F0) * pow(1.0 - cosVN, 5.0);;

    float3 kd = lerp(1.0 - F, 0.0, metalness);
    float3 diffuseIBL = kd * albedo * irradiance;
    uint width, height, levels;
    txSpecular.GetDimensions(0, width, height, levels);
    uint specularuvtureLevels = levels;
    float3 specularIrradiance = txSpecular.SampleLevel(samLinear, LightReflection, roughness * specularuvtureLevels).rgb;
    float2 specularBRDF = specularBRDF_LUT.Sample(samLinear, float2(cosVN, roughness)).rg;
    float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
    ambientLighting = diffuseIBL + specularIBL;


    float4 PosShadow = deferredShadowPosition.Sample(samLinear, pin.uv);
    float currentShadowDepth = PosShadow.z;
    float2 shadowUV = PosShadow.xy ;
    shadowUV.y = -shadowUV.y;
    shadowUV = shadowUV * 0.5 + 0.5;
    float shadowFactor = 1.0f;
    if (shadowUV.x >= 0.0 && shadowUV.x <= 1.0 && shadowUV.y >= 0.0 && shadowUV.y <= 1.0)
    {
        float2 offset[9] =
        {
            float2(-1, -1), float2(0, -1), float2(1, -1),
			float2(-1, 0), float2(0, 0), float2(1, 0),
			float2(-1, 1), float2(0, 1), float2(1, 1)
        };
        uint width, height, levels;
        txShadow.GetDimensions(width, height);
        float texelSize = 1.0 / 8192; //텍셀 크기
        shadowFactor = 0.f;
    	[unroll]
        for (int i = 0; i < 9; i++)
        {
            float2 sampleUV = shadowUV + offset[i] * texelSize; //오프셋 계산
                //sampleCmpLevelZero로 pcf샘플링
            shadowFactor += txShadow.SampleCmpLevelZero(samplerComparison, sampleUV, currentShadowDepth - 0.001);

        }
        shadowFactor = shadowFactor / 9.f;
            

    }


    float4 final;
    if (mat.metalness.y >0.5)
    	final = float4(pow(float3(shadowFactor * (directLighting + ambientLighting)), 1.0 / 2.2), 1.0);
	else
        final = float4(pow(float3((shadowFactor * directLighting)), 1.0 / 2.2), 1.0);
		

    float4 emissive = deferredEmissive.Sample(samLinear, pin.uv);
    if (length(emissive))
        final += emissive;

    return final;
}
