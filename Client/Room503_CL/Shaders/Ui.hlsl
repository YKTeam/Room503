#include "Common.hlsl"


Texture2D    gDiffuseMap : register(t0);

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
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
	VertexOut vout;
	
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;

	vout.PosH = mul(posW, gViewProj);

	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, gMatTransform).xy;

	return vout;
}


float4 PS(VertexOut pin) : SV_Target
{
	float4 base = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC) ;

	//float4 diffuseAlbedo = saturate(base) * gDiffuseAlbedo;

	return base;
//float4 base = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC);
}
