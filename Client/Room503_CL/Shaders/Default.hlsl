

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"
#include "Common.hlsl"

Texture2D    gDiffuseMap : register(t0);
Texture2D    gDetailMap : register(t1);

struct VertexIn
{
	float3 PosL    : POSITION;  
    float3 NormalL : NORMAL;
	float2 TexC0 : TEXCOORD0;
	float2 TexC1 : TEXCOORD1;

#ifdef SKINNED
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
#endif
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
	
#ifdef SKINNED
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.BoneWeights.x;
	weights[1] = vin.BoneWeights.y;
	weights[2] = vin.BoneWeights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}

	vin.PosL = posL;
	vin.NormalL = normalL;
#endif

    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.

    vout.NormalW =  mul(vin.NormalL, (float3x3)gWorld);
    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
	
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

    float4 diffuseAlbedo =  saturate((base * 0.5f) + (detail * 0.5f)) * gDiffuseAlbedo;

#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon 
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif
    // Interpolating normal can unnormalize it, so renormalize it.
	//diffuseAlbedo = float4(diffuseAlbedo * diffuse,1);

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
#ifdef FOG
	/*float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);*/
#endif
	
    // Common convention to take alpha from diffuse albedo.
	
    litColor.a = diffuseAlbedo.a;
	

    return litColor;
}


