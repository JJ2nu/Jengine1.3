Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
TextureCube txSpecular : register(t2);

Texture2D txEmissive : register(t3);
Texture2D txOpacity : register(t4);
TextureCube txEnv : register(t5);
Texture2D txMetalness : register(t6);
Texture2D txRoughness : register(t7);

TextureCube txIrridiance : register(t8);
Texture2D specularBRDF_LUT : register(t9);
Texture2D txShadow : register(t10);


SamplerState samLinear : register(s0);
//SamplerState defaultSampler : register(s1);
//SamplerState spBRDF_Sampler : register(s2);
SamplerComparisonState samplerComparison : register(s3);

//deferred
Texture2D deferredAlbedo : register(t12);
Texture2D deferredNormal : register(t13);
Texture2D deferredMaterial : register(t14);
Texture2D deferredEmissive : register(t15);
Texture2D deferredShadowPosition : register(t16);




static const float3 Fdielectric = 0.04;
static const float PI = 3.141592;
static const float Epsilon = 0.00001;

static const float coefficients[55] =
{
    0.0165967f, 0.0169504f, 0.0172961f, 0.0176334f, 0.0179618f, 0.0182807f,
    0.0185895f, 0.0188876f, 0.0191745f, 0.0194496f, 0.0197124f, 0.0199623f,
    0.0201987f, 0.0204211f, 0.0206291f, 0.0208222f, 0.0210001f, 0.0211623f,
    0.0213085f, 0.0214384f, 0.0215516f, 0.0216478f, 0.0217268f, 0.0217883f,
    0.0218322f, 0.0218583f, 0.0218665f, 0.0218568f, 0.0218291f, 0.0217834f,
    0.0217197f, 0.0216381f, 0.0215386f, 0.0214213f, 0.0212863f, 0.0211337f,
    0.0209637f, 0.0207764f, 0.0205721f, 0.0203509f, 0.0201132f, 0.0198592f,
    0.0195892f, 0.0193036f, 0.0190028f, 0.0186871f, 0.0183569f, 0.0180128f,
    0.0176552f, 0.0172845f, 0.0169013f, 0.0165061f, 0.0160994f, 0.0156818f,
    0.0152539f
};
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 EyePos;

}

struct Light
{
    float4 lightDir;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    float4 lightSource;
    float4 radiance;

};
struct Material
{
    float4 metalness;
    float4 roughness;
    float4 specular;
    float4 emissive;
    float4 specularExponent;

};
cbuffer LightBuffer : register(b1)
{
    Light light[4];
    Material mat;
    bool flag1 = false;
    float3 pad;
    bool flag2 = false;
    float3 pad2;
}
#ifdef Skinning
cbuffer MatrixPallete :register(b1)
{
    matrix MatrixPalleteArray[128];
}
#endif
cbuffer ShadowTransform : register(b2)
{
    matrix ShadowView;
    matrix ShadowProjection;
}

cbuffer DefferedConstant : register(b4)
{
    matrix InverseViewMatrix;
    matrix InverseProjectionMatrix;
    float2 nearFar;
    float2 padding;
}

cbuffer BillboardAnimConstant :register(b5)
{
//x : widthcnt, y : heightcnt, z : current index, w : max cnt
    float4 frameCnt;
    float4 colorBlend;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Tex : TEXCOORD0;
#ifdef Skinning
    int4 BlendIndices : BLENDINDICES;
    float4 BlendWeight : BLENDWEIGHTS;
#endif
};

struct PARTICLE_VS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 Tex : TEXCOORD0;
    uint InstanceID : SV_InstanceID; // 인스턴스 ID 추가
};



struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 PosShadow : TEXCOORD1;
};
struct PARTICLE_PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 PosShadow : TEXCOORD1;
    float4 ColorBlend : TEXCOORD2;
};

struct DeferredOut
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Material : SV_Target2;
    float4 Emissive : SV_Target3;
    float4 ShadowPosition : SV_Target4;
};

struct QUAD_VS_INPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
};
struct QUAD_PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 positionShadow : POSITION;
};
float luminance(float3 v)
{
    return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}
float3 change_luminance(float3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}
float3 ToneMap(float3 v, float max_white_l)
{
    float l_old = luminance(v);
    float numerator = l_old * (1.f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.f + l_old);
    return change_luminance(v, l_new);
}



