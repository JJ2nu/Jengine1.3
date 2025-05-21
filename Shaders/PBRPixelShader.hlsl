#include "PBRShared.fxh"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
    uint width, height, levels;
    txSpecular.GetDimensions(0, width, height, levels);
    return levels;
}

float4 main(PS_INPUT pin) : SV_Target
{
    float3 directLighting = float3(0.f, 0.f, 0.f);

    float3 rough = txRoughness.Sample(samLinear, pin.Tex);
    float roughness;
    if (length(rough))
        roughness = rough.r * mat.roughness.x;
    else
        roughness = mat.roughness.x;
	
    float roughnessSqaure = pow(max(0.01, roughness), 2.0f);

    float3 metal = txMetalness.Sample(samLinear, pin.Tex);
    float metalness;
    if (length(metal))
        metalness = metal.r * mat.metalness.x;
    else
        metalness = mat.metalness.x;

    float3 albedo =txDiffuse.Sample(samLinear, pin.Tex);
	if (length(albedo))
		albedo = pow(albedo.rgb, 2.2);
	else
        albedo = mat.specular.rgb;
    float4 opacity = txOpacity.Sample(samLinear, pin.Tex);




    float3 N = txNormal.Sample(samLinear, pin.Tex).rgb;
    if (length(N))
    {
		N = normalize(2.0 * N - 1.0);
        float3x3 tangentBasis = float3x3(pin.Tangent, pin.Binormal, pin.Norm);
		N = normalize(mul(N, tangentBasis));
    }
	else
        N = pin.Norm;


    float3 ViewVector = normalize(EyePos.xyz - pin.WorldPos.xyz);
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
    uint specularTextureLevels = levels;
    float3 specularIrradiance = txSpecular.SampleLevel(samLinear, LightReflection, roughness * specularTextureLevels).rgb;
    float2 specularBRDF = specularBRDF_LUT.Sample(samLinear, float2(cosVN, roughness)).rg;
    float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
    ambientLighting = diffuseIBL + specularIBL;


    float currentShadowDepth = pin.PosShadow.z / pin.PosShadow.w;
    float2 shadowUV = pin.PosShadow.xy / pin.PosShadow.w;
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




    float4 final = float4(pow(float3((shadowFactor * directLighting) + (ambientLighting * mat.metalness.y)), 1.0 / 2.2), 1.0);

    //float4 emissive = txEmissive.Sample(samLinear, pin.Tex);
    //if (length(emissive))
    //    final += emissive;

    //if (length(opacity))
    //    final.a = opacity.a;
    //else
    //    final.a = 1.0f;
    //if (final.a < 0.01)
    //    discard;

    
    
    
    
    float3 V = EyePos - pin.WorldPos;
    V = normalize(V);
    float intensity = 0.5f;
    float factor = pow(max(0, 1 - (dot(V, N))), intensity);
   
    final = factor * float4(0, 0.3, 0.6, 0.5f);
    final.a = 1;
    return final;
    
    
}


