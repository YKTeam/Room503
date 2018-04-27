

// Include structures and functions for lighting.
#include "Common.hlsl"

Texture2D    gDiffuseMap : register(t0);
Texture2D    gNomalMap : register(t2);

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
#ifdef SKINNED
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
#endif
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float4 ShadowPosH : POSITION0;
	float3 PosW    : POSITION1;
    float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC    : TEXCOORD;
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
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
		tangentL += weights[i] * mul(vin.TangentL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}

	vin.PosL = posL;
	vin.NormalL = normalL;
	vin.TangentL = tangentL;
#endif

    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW =  mul(vin.NormalL, (float3x3)gWorld);

	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	//float4 texC2 = mul(float4(vin.TexC1, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, gMatTransform).xy;
	//vout.TexC1 = mul(texC2, gMatTransform).xy;

	// Generate projective tex-coords to project shadow map onto scene.
	vout.ShadowPosH = mul(posW, gShadowTransform);
    return vout;
}


float4 PS(VertexOut pin) : SV_Target
{
	//gDetailMap
	uint diffuseTexIndex = DiffuseMapIndex;
	//float4 green = float4(0, 1, 0, 1);
	float4 base = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC);
	//float4 detail = gDetailMap.Sample(gsamLinearWrap, pin.TexC1);

	//법선 보간하면 단위길이가 안될 수 있어서 다시
	pin.NormalW = normalize(pin.NormalW);
	float4 normalMapSample = gNomalMap.Sample(gsamAnisotropicWrap, pin.TexC);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
	// 법선 매핑 온오프.
	//bumpedNormalW = pin.NormalW;
    
	float4 diffuseAlbedo =  saturate(base) * gDiffuseAlbedo;

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

	// Only the first light casts a shadow.
	
	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);

    const float shininess = (1.0f - gRoughness)*normalMapSample.a;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };

	float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
		bumpedNormalW, toEyeW, shadowFactor, 5);

    float4 litColor = ambient + directLight;


#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif
	
    litColor.a = diffuseAlbedo.a;
	

    return litColor;
}


