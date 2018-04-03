

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"
#include "Common.hlsl"

Texture2DArray gTreeMapArray : register(t0);

struct VertexIn
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
};

struct VertexOut
{
	float3 CenterW : POSITION;
	float2 SizeW   : SIZE;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC    : TEXCOORD;
    uint   PrimID  : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.CenterW = vin.PosW;
	vout.SizeW   = vin.SizeW;

	return vout;
}
 
// 각 점을 사가형 으로 확장. 프리미티브 호충당 최대 출력 정점 개수 4
[maxvertexcount(4)]
void GS(point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream)
{
	//
	// 빌보드가 xz평면에 붙어서 y방향으로 세워진 상태에서 카메라를 향하게 만드는 세계공간기준 빌보드 국소 좌표계
	// 계산
	//
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = gEyePosW - gin[0].CenterW;
	look.y = 0.0f; // xz평면에 투영
	look = normalize(look);
	float3 right = cross(up, look);

	//
	// 세계공간 기준의 삼각형 띠 정점들을 계산 (사각형 구성하는 삼각형)
	//
	float halfWidth = 0.5f*gin[0].SizeW.x;
	float halfHeight = 0.5f*gin[0].SizeW.y;

	float4 v[4];
	if (primID % 5 == 0) halfHeight *= 1.2f;
	else if (primID % 11 == 0) {
		halfHeight *= 1.4f;
		halfWidth *= 1.4f;
	}
	v[0] = float4(gin[0].CenterW + halfWidth*right - halfHeight*up, 1.0f); //오른쪽아래
	v[1] = float4(gin[0].CenterW + halfWidth*right + halfHeight*up, 1.0f); //오른쪽위;
	v[2] = float4(gin[0].CenterW - halfWidth*right - halfHeight*up, 1.0f); //왼쪽아래
	v[3] = float4(gin[0].CenterW - halfWidth*right + halfHeight*up, 1.0f);

	//
	// 사각형 정점들을 세계 공간으로 변환하고 
	// 그것들을 하나의 삼각형 띠로 출력
	//
	float2 texC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	GeoOut gout;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		gout.PosH = mul(v[i], gViewProj);
		gout.PosW = v[i].xyz;
		gout.NormalW = look;
		gout.TexC = texC[i];
		gout.PrimID = primID;

		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin) : SV_Target
{
	float3 uvw = float3(pin.TexC, pin.PrimID%3);
    float4 diffuseAlbedo = gTreeMapArray.Sample(gsamAnisotropicWrap, uvw) * gDiffuseAlbedo;
	
#ifdef ALPHA_TEST
	// 텍스처 알파가 0.3보다 작으면 픽셀 폐기. 셰이더 안에서
	// 이 판정을 최대한 일찍 수행하는 것이 바람직함.
	clip(diffuseAlbedo.a - 0.3f);
#endif

	// 법선을 보간 후 다시 정규화
	pin.NormalW = normalize(pin.NormalW);

	// 조명 되는 점에서 눈으로의 벡터
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize

	 // 조명계산들
    float4 ambient = gAmbientLight*diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;

    float4 litColor = ambient ;

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}


