
// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

Texture2D    gDiffuseMap : register(t0);
Texture2D    gDetailMap : register(t1);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
	uint gTextureAniIndex;
	uint Pad0;
	uint Pad1;
	uint Pad2;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gViewMini;
	float4x4 gProjMini;
	float4x4 gViewProjMini;
	float3 gEyePosW;
	float cbPerObjectPad1;
	float3 gEyePosWMini;
	float cbPerObjectPad2;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;

	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 cbPerObjectPad3;

	Light gLights[MaxLights];
};

cbuffer cbMaterial : register(b2)
{
	float4   gDiffuseAlbedo;
	float3   gFresnelR0;
	float    gRoughness;
	float4x4 gMatTransform;
	uint     DiffuseMapIndex;
};

struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC0 : TEXCOORD0;
	float2 TexC1 : TEXCOORD1;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 TexC0    : TEXCOORD0;
	float2 TexC1    : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProjMini);
	
	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC0, 0.0f, 1.0f), gTexTransform);
	float4 texC2 = mul(float4(vin.TexC1, 0.0f, 1.0f), gTexTransform);
	vout.TexC0 = mul(texC, gMatTransform).xy;
	vout.TexC1 = mul(texC2, gMatTransform).xy;
    return vout;
}


float4 PS(VertexOut pin) : SV_Target
{
	//gDetailMap
	uint diffuseTexIndex = DiffuseMapIndex;
	//float4 green = float4(0, 1, 0, 1);
	float4 base = gDiffuseMap.Sample(gsamLinearWrap, pin.TexC0);
	float4 detail = gDetailMap.Sample(gsamLinearWrap, pin.TexC1);
	//float4 mycolor = float4(0.85, 1, 0.85,1);
    float4 diffuseAlbedo =  saturate((base * 0.5f) + (detail * 0.5f)) * gDiffuseAlbedo;
	
#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon 
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif

    // Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize

    // Light terms.
    float4 ambient = gAmbientLight*diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor, 5);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}


