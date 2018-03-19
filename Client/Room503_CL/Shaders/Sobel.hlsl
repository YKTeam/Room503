//=============================================================================
// Performs edge detection using Sobel operator.
//=============================================================================

Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);


// 여러 파장의 빛에 대한 눈의 민감도에 기초한 휘도 근사값
float CalcLuminance(float3 color)
{
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

[numthreads(16, 16, 1)]
void SobelCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // 픽셀 주변 이웃 픽셀들을 추출
	float4 c[3][3];
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			int2 xy = dispatchThreadID.xy + int2(-1 + j, -1 + i);
			c[i][j] = gInput[xy]; 
		}
	}

	// 각 색상 채널에 대해, 소벨 핵을 이용해 x y의 편미분 계수를 추정한다.
	float4 Gx = -1.0f*c[0][0] - 2.0f*c[1][0] - 1.0f*c[2][0] + 1.0f*c[0][2] + 2.0f*c[1][2] + 1.0f*c[2][2];

	float4 Gy = -1.0f*c[2][0] - 2.0f*c[2][1] - 1.0f*c[2][1] + 1.0f*c[0][0] + 2.0f*c[0][1] + 1.0f*c[0][2];

	// gx,gy가 이 픽셀의 기술기가 된다. 각 색상 채널에 대해 변화율이 최대가 되는 크기를 구한다.
	float4 mag = sqrt(Gx*Gx + Gy*Gy);

	// 윤곽선 픽셀은 검게, 아니면 희게 만든다.
	mag = 1.0f - saturate(CalcLuminance(mag.rgb));
	if (mag.r <= 0.5f || mag.g <= 0.5f || mag.b <= 0.5f) mag = float4(0, 0, 0, 1);
	gOutput[dispatchThreadID.xy] = mag;
}
