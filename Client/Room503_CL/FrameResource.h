#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT TextureAniIndex;
	UINT Pad0;
	UINT Pad1;
	UINT Pad2;
};

struct SkinnedConstants
{
	DirectX::XMFLOAT4X4 BoneTransforms[96];
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();

	/*DirectX::XMFLOAT4X4 ViewMini = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ProjMini = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProjMini = MathHelper::Identity4x4();*/

    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
	//DirectX::XMFLOAT3 EyePosWMini = { 0.0f, 0.0f, 0.0f };
	//float cbPerObjectPad2 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	DirectX::XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	float gFogStart = 50.0f;
	float gFogRange = 700.0f;
	DirectX::XMFLOAT2 cbPerObjectPad3;


	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
};

struct SkinnedVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;

	DirectX::XMFLOAT3 BoneWeights;
	BYTE BoneIndices[4];
};

// CPU가 한 프레임의 명령 목록들을 구축하는 데 필요한 자원들을 대표하는 클래스
// 응용 프로그램마다 필요한 자원이 다를 것이므로,
// 이런 클래스의 멤버 구성 역시 응용 프로그램마다 달라야 할 것
struct FrameResource
{
public:
    
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount, UINT materialCount);//, UINT waveVertCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // 명령 할당자는 GPU가 명령들을 다 처리한 후 재설정해야 한다.
	// 따라서 프레임마다 할당자가 필요하다
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // 상수 버퍼는 그것을 참조하는 명령들을 GPU가 다 처리한 후에 갱신해야 한다.
	// 프레임마다 상수 버퍼를 새로 만들어야 한다.
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;  //이 버퍼는 하나의 렌더링 패스 전체에서 변하지 않는 상수 자료 저장( 시점위치, 시야행렬,투영행렬, 화면크기)
    std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	std::unique_ptr<UploadBuffer<SkinnedConstants>> SkinnedCB = nullptr;
    // 펜스는 현재 울타리 지점까지의 명령들을 표시하는 값이다.
	// 이 값은 GPU가 아직 이프레임 자원들을 사용하고 있는지
	// 판정하는 용도로 쓰인다.
    UINT64 Fence = 0;
};