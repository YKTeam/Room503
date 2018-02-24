//=============================================================================
// 최대 5픽셀까지의 흐리기 반지름으로 분리 가능 가우스 흐리기 수행
//=============================================================================

cbuffer cbSettings : register(b0)
{
	// 루트 상수들에 대응되는 상수 버퍼에 대열을 둘수없어서 일일이 나열
	int gBlurRadius;

	// 11개의 흐리기 가중치
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

static const int gMaxBlurRadius = 5;


Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256
#define CacheSize (N + 2*gMaxBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 groupID : SV_GroupID,
				int3 dispatchThreadID : SV_DispatchThreadID,
				int groupIndexID : SV_GroupIndex)
{
	// 가중치를 색인으로 쓸수 있게 배열에 넣었음
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// 대역폭을 줄이려고 지역 스레드 저장소를 채움
	// 흐리기 반지름 때문에, 픽셀 N개를 흐리려면 N+ 2*blurRadius개의 픽실을 읽음
	//
	
	// 스레드 그룹은 N개의 스레드를 실행한다.
	// 여분의 픽셀 2*blurRadius개를 위해 그만큼의 스레드가 픽셀을 하나 더 추출하게 함
	if(groupThreadID.x < gBlurRadius)
	{
		// 이미지 가장자리 바깥의 표본을 이미지 가장자리 표본으로 한정한다.
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N-gBlurRadius)
	{
		// 마찬가지로 가장자리 처리를 한다
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x-1);
		gCache[groupThreadID.x+2*gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}
	gCache[groupThreadID.x+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];
	// 동기화 처리.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// 픽셀 흐리기 알고리즘
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;
		
		//if (groupID.x % 2 == 0)
		blurColor += weights[i+gBlurRadius]*gCache[k];
	}

	gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 groupID : SV_GroupID,
				int3 dispatchThreadID : SV_DispatchThreadID,
				int groupIndexID : SV_GroupIndex)
{
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	
	if(groupThreadID.y < gBlurRadius)
	{
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N-gBlurRadius)
	{
		int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y-1);
		gCache[groupThreadID.y+2*gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}
	
	gCache[groupThreadID.y+gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];


	GroupMemoryBarrierWithGroupSync();
	
	//
	// 흐리기
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;
		
		blurColor += weights[i+gBlurRadius]*gCache[k];
		//blurColor += weights[gBlurRadius] * gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}