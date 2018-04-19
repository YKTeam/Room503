

#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "GameObject.h"
#include "BlurFilter.h"
#include "SobelFilter.h"
#include "RenderTarget.h"
#include <tchar.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")


const int gNumFrameResources = 2;

enum class RenderLayer : int
{
	Opaque = 0,
	Grid,
	Player,
	Shadow,
	Grass,
	SkyBox,
	Enemy,//적
	CollBox,
	Count
};



class MyScene : public D3DApp
{
public:
	MyScene(HINSTANCE hInstance);
	MyScene(const MyScene& rhs) = delete;
	MyScene& operator=(const MyScene& rhs) = delete;
	~MyScene();

	virtual bool Initialize()override;

private:
	virtual void CreateRtvAndDsvDescriptorHeaps()override;
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
	//virtual void DrawMini(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	//void UpdateInstanceData(const GameTimer& gt);
	void UpdateSkinnedCBs(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	//void BuildConstantBuffers();
	void LoadTextures();
	void BuildRootSignature();
	void BuildPostProcessRootSignature();
	void BuildPostProcessSobelRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	
	void BuildLandGeometry();
	void BuildCollBoxGeometry(Aabb colbox,const std::string geoName, const std::string meshName);
	void BuildFbxGeometry(const std::string fileName, const std::string geoName, const std::string meshName, float loadScale, bool isMap, bool hasAniBone); //본갯수리턴
	void BuildAnimation(const std::string fileName, const std::string clipName, float loadScale, bool isMap);

	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildGameObjects();
	void DrawGameObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<GameObject*>& ritems , const int itemState);
	void DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	UINT mCbvSrvDescriptor2Size = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mPostProcessRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mPostProcessSobelRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mCbvSrvUavDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, Aabb> mBounds;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mFlareSpriteInputLayout;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mSkyBoxInputLayout;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> mMinimapInputLayout;

	// List of all the render items.
	std::vector<std::unique_ptr<GameObject>> mAllRitems;
	std::vector<GameObject*> mOpaqueRitems[(int)RenderLayer::Count];
	std::unique_ptr<BlurFilter> mBlurFilter;
	std::unique_ptr<SobelFilter> mSobelFilter = nullptr;
	std::unique_ptr<RenderTarget> mOffscreenRT = nullptr;

	bool mFrustumCullingEnabled = true;
	BoundingFrustum mCamFrustum;

	PassConstants mMainPassCB;

	//스키닝 애니메이션데이터 
	std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
	SkinnedData mSkinnedInfo;
	Subset mSkinnedSubsets;
	//메쉬랑 본오프셋이랑 
	aiMesh* playerMesh;
	std::vector<XMFLOAT4X4> playerboneOffsets;
	std::vector<pair<string, int>> playerboneIndexToParentIndex;
	vector<pair<std::string, int>> playerboneName;

	bool mIsWireframe = false;
	float mSunTheta = 205.917;

	//구조가 생성자로 생성되는구조임
	ObjectConstants mPlayerInfo;
	int m_ObjIndex = 0;
	int m_BlurCount = 0;
	int m_CameraMoveLevel = 0;
	const float m_CplayerSpeed = 75;
	float m_PlayerSpeed = 50;
	BOOL isInside = false;
	float mx = 0, my= 0;


	//Camera mCameraTmp;
	Camera mCamera;
	
	POINT mLastMousePos;

	//LoadModel* modelImport;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// 디버그 빌드에서는 실행시점 메모리 점검 기능을 켠다
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		MyScene MyScene(hInstance);

		if (!MyScene.Initialize())
			return 0;

		return MyScene.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

MyScene::MyScene(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
}

MyScene::~MyScene()
{
}

bool MyScene::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//md3dDevice->

#ifdef _DEBUG
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")
#endif

	mBlurFilter = std::make_unique<BlurFilter>(md3dDevice.Get(),
		mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	mSobelFilter = std::make_unique<SobelFilter>(
		md3dDevice.Get(),
		mClientWidth, mClientHeight,
		mBackBufferFormat);
	
	mOffscreenRT = std::make_unique<RenderTarget>(
		md3dDevice.Get(),
		mClientWidth, mClientHeight,
		mBackBufferFormat);

	//BuildConstantBuffers();
	LoadTextures();
	BuildRootSignature();
	BuildPostProcessRootSignature();
	BuildPostProcessSobelRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildFbxGeometry("Model/robotFree3.fbx", "robot_freeGeo", "robot_free", 1.0f, false , true);//angle  robotModel  robotIdle
	BuildFbxGeometry("Model/Robot Kyle.fbx", "robotGeo", "robot" , 0.1f, false , false);
	BuildFbxGeometry("Model/testmap2.obj", "map00Geo", "map00", 1, true, false);
	//BuildAnimation("Model/robotidle2.fbx","idle", 1.0f, false);
	BuildAnimation("Model/robotFree3.fbx","walk", 1.0f, false);//robotwalk
	

	BuildMaterials();
	BuildGameObjects();
	BuildFrameResources();

	BuildPSOs();

	//mCamera.LookAt(mCamera.GetPosition3f(), XMFLOAT3(mPlayerInfo.World._41, mPlayerInfo.World._42, mPlayerInfo.World._43), mCamera.GetLook3f());
	/*XMVECTOR x = XMVectorSet(0,0,0,0);
	XMVECTOR y = XMVectorSet(0, 1, 0, 0);
	mCamera.LookAt(mCamera.GetPosition(), x, y);*/
	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();
	
	return true;
}

void MyScene::CreateRtvAndDsvDescriptorHeaps()
{
	// Add +1 descriptor for offscreen render target.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void MyScene::OnResize()
{
	if (mSobelFilter != nullptr)
	{
		mSobelFilter->OnResize(mClientWidth, mClientHeight);
	}
	if (mBlurFilter != nullptr)
	{
		mBlurFilter->OnResize(mClientWidth, mClientHeight);
	}
	if (mOffscreenRT != nullptr)
	{
		mOffscreenRT->OnResize(mClientWidth, mClientHeight);
	}
	mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 10000.0f);

	mCamera.UpdateViewMatrix();

	BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera.GetProj());

	D3DApp::OnResize();
}

void MyScene::Update(const GameTimer& gt)
{
	
	OnKeyboardInput(gt);

	// 순환적으로 자원 프레임 배열의 다음 원소에 접근한다
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	// GPU가 현재 프레임 자원의 명령들을 다 처리했는지 확인한다. 아직
	// 다 처리하지 않았으면 GPU가 이 울타리 지점까지의 명령들을 처리할 때까지 기다린다
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	

	AnimateMaterials(gt);
	UpdateSkinnedCBs(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialBuffer(gt);
	UpdateMainPassCB(gt);
}

void MyScene::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
    // 명령 기록에 관련된 메모리의 재활용을 위해 명령 할당자를 재설정한다.
    // 재설정은 GPU가 관련 명령 목ㅇ록들을 모두 처리한 후에 일어남
	ThrowIfFailed(cmdListAlloc->Reset());

	// 명령 목록을 ExecuteCommandList를 통해서 명령 대기열에
	// 추가했다면 명령 목록을 재설정할 수 있다. 명령 목록을
	// 재설정하면 메모리가 재활용된다
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvSrvUavDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // 뷰포트와 가위 직사각형 설정
	// 명령 목록을 재설정할 때마다 재설정
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 자원 용도에 관련된 상태 전이를 Direct3D에 통지한다
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOffscreenRT->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // 후면 버퍼와 깊이 버퍼를 지운다.
	mCommandList->ClearRenderTargetView(mOffscreenRT->Rtv(), Colors::LightGray, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
    // 렌더링 결과가 기록될 렌더 대상 버퍼들을 지정
	mCommandList->OMSetRenderTargets(1, &mOffscreenRT->Rtv(), true, &DepthStencilView());

	////////////////////////////////////////////////////////////
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());


	if (mIsWireframe)
	{
		mCommandList->SetPipelineState(mPSOs["opaque_wireframe"].Get());
	}
	else
	{
		mCommandList->SetPipelineState(mPSOs["grid"].Get());
	}
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Grid], (int)RenderLayer::Grid);
	
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Enemy], (int)RenderLayer::Enemy);
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::CollBox], (int)RenderLayer::CollBox);
	//
	//플레이어..
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	
	if (mIsWireframe)
	{
		mCommandList->SetPipelineState(mPSOs["opaque_wireframe"].Get());
	}
	else
	{
		mCommandList->SetPipelineState(mPSOs["skinnedOpaque"].Get());
	}
	DrawGameObjects(mCommandList.Get(), mOpaqueRitems[(int)RenderLayer::Player], (int)RenderLayer::Player);
	

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOffscreenRT->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	mSobelFilter->Execute(mCommandList.Get(), mPostProcessSobelRootSignature.Get(),
		mPSOs["sobel"].Get(), mOffscreenRT->Srv());
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	
	mCommandList->SetGraphicsRootSignature(mPostProcessSobelRootSignature.Get());
	mCommandList->SetPipelineState(mPSOs["composite"].Get());
	mCommandList->SetGraphicsRootDescriptorTable(0, mOffscreenRT->Srv());
	mCommandList->SetGraphicsRootDescriptorTable(1, mSobelFilter->OutputSrv());
	DrawFullscreenQuad(mCommandList.Get());

	//blur 
	//int blurLevel = m_BlurCount;//+ m_CameraMoveLevel;
	////if (m_BlurCount + m_CameraMoveLevel >= 2) blurLevel = 2;
	//mBlurFilter->Execute(mCommandList.Get(), mPostProcessRootSignature.Get(),
	//	mPSOs["horzBlur"].Get(), mPSOs["vertBlur"].Get(), CurrentBackBuffer(), blurLevel);
	//// Prepare to copy blurred output to the back buffer.
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
	//	D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	//mCommandList->CopyResource(CurrentBackBuffer(), mBlurFilter->Output());

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	
	
	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// 명령 실행을 위해 명령 목록을 명령 대기열에 추가한다.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 후면 버퍼와 전면 버퍼 교환
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// 현재 울타리 지점까지의 명령들을 표시하도록 울타리 값을 전진시킨다.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// 새 울타리 지점을 설정하는 명령을 명령 대기열에 추가한다.
	// 지금 우리는 GPU 시간선 상에 있으므로, 새 울타리 지점은 GPU가
	// 이 시크날 명령 이전까지의 모든 명령을 처리하기 전까지는 설정되지 않는다
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void MyScene::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
	}
	SetCapture(mhMainWnd);
}

void MyScene::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_CameraMoveLevel = 0;
	ReleaseCapture();
}

void MyScene::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		
	}
	if ((btnState & MK_RBUTTON) != 0)
	{
		float blurMx = 0.0f;
		float blurMy = 0.0f;
		mx = 0;
		my = 0;
		// Make each pixel correspond to a quarter of a degree.
		mx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		my = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));
		blurMx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		blurMy = 0.1f*static_cast<float>(y - mLastMousePos.y);
		printf("%.2f, %.2f\n", mx,my);
		if ( abs( blurMx ) <= 0.3f && abs(blurMy) <= 0.3f)
			m_CameraMoveLevel = m_BlurCount;
		else if (abs(blurMx) >= 0.7f || abs(blurMy) >= 0.7f) {
			m_CameraMoveLevel = 2;
		}
		else if (abs(blurMx)  > 0.3f || abs(blurMy) > 0.3f)
			m_CameraMoveLevel = 1+ m_BlurCount;
		
		
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
	//mCamera.UpdateViewMatrix();
	
}


void MyScene::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState(VK_F2) & 0x8000) {
		OnResize();
		return;
	}
	if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		OnResize();
		return;
	}
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		OnResize();
		return;
	}

	if (GetAsyncKeyState('1') & 0x8000) mIsWireframe = true;
	else mIsWireframe = false;

	if (GetAsyncKeyState('Q') & 0x8000) {
		mSunTheta += gt.DeltaTime();
		printf("%.3f\n", (mSunTheta));
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		m_BlurCount = 1;
	}
	else {
		m_BlurCount = 0;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
	}

	if (GetAsyncKeyState('W') & 0x8000) {
		mCamera.Walk(1000.0f *dt);
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		mCamera.Walk(-1000.0f *dt);
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		mCamera.Strafe(-1000.0f *dt);
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		mCamera.Strafe(1000.0f *dt);
	}

	if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN)) {
		mSkinnedModelInst->SetNowAni("walk");
		float walkSpeed = -150;
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			eplayer[0]->SetLook3f(XMFLOAT3(1, 0, 0));
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			if ( !( GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) ))
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			eplayer[0]->SetLook3f(XMFLOAT3(-1, 0, 0));
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			if ( !(GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN) ))
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			XMFLOAT3 look = Vector3::Normalize( Vector3::Add( XMFLOAT3(0, 0, -1) , eplayer[0]->GetLook3f()));
			
			eplayer[0]->SetLook3f(look);
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			auto& eplayer = mOpaqueRitems[(int)RenderLayer::Player];
			XMFLOAT3 look = Vector3::Normalize(Vector3::Add(XMFLOAT3(0, 0, 1), eplayer[0]->GetLook3f()));

			eplayer[0]->SetLook3f(look);
			eplayer[0]->SetRight3f(Vector3::CrossProduct(eplayer[0]->GetUp3f(), eplayer[0]->GetLook3f(), true));
			eplayer[0]->SetPosition(Vector3::Add(eplayer[0]->GetPosition(), Vector3::ScalarProduct(eplayer[0]->GetLook3f(), walkSpeed *dt, false)));
		}
	}
	else mSkinnedModelInst->SetNowAni("idle");
	

	mCamera.UpdateViewMatrix();
}

void MyScene::AnimateMaterials(const GameTimer& gt)
{

}

void MyScene::UpdateSkinnedCBs(const GameTimer& gt)
{
	auto currSkinnedCB = mCurrFrameResource->SkinnedCB.get();

	mSkinnedModelInst->UpdateSkinnedAnimation(gt.DeltaTime());

	SkinnedConstants skinnedConstants;
	std::copy(
		std::begin(mSkinnedModelInst->FinalTransforms),
		std::end(mSkinnedModelInst->FinalTransforms),
		&skinnedConstants.BoneTransforms[0]);

	currSkinnedCB->CopyData(0, skinnedConstants);
}

void MyScene::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();

	for (auto& e : mOpaqueRitems[(int)RenderLayer::Player])
	{
		auto enemy = mOpaqueRitems[(int)RenderLayer::Enemy];
		auto rand = mOpaqueRitems[(int)RenderLayer::Grid];
		mCamera.Pitch(my);
		mCamera.RotateY(mx);

		//땅 -값
		if (e->GetPosition().y > rand[0]->GetPosition().y + 25)//모델키는 로드시 스케일로 맞춰서 일단 상수로
			e->GravityUpdate(gt);
		else
			e->SetPosition(XMFLOAT3(e->GetPosition().x, -25, e->GetPosition().z));

		if (e->bounds.IsCollsionAABB(e->GetPosition(),&enemy[0]->bounds, enemy[0]->GetPosition()))
			printf("충돌 \n");
		else 
			printf("NO \n");

		//회전초기화
		mx = 0;
		my = 0;

		e->NumFramesDirty = gNumFrameResources;
		
		mCamera.UpdateViewMatrix();
		//mCameraTmp = mCamera;
	}
	
	for (auto& e : mOpaqueRitems[(int)RenderLayer::Enemy])
	{

		auto rand = mOpaqueRitems[(int)RenderLayer::Grid];
		if (e->GetPosition().y > rand[0]->GetPosition().y + 25)//모델키는 로드시 스케일로 맞춰서 일단 상수로
			e->GravityUpdate(gt);
		else
			e->SetPosition(XMFLOAT3(e->GetPosition().x, -25, e->GetPosition().z));

		e->NumFramesDirty = gNumFrameResources;
	}

	for (auto& e : mOpaqueRitems[(int)RenderLayer::CollBox])
	{
		auto player = mOpaqueRitems[(int)RenderLayer::Player];
		auto enemy = mOpaqueRitems[(int)RenderLayer::Enemy];
		if (e->Geo->Name == "robot_freeBoxGeo") {
			e->World = player[0]->World;
		}
		else {
			e->World = enemy[0]->World;
		}

		e->NumFramesDirty = gNumFrameResources;
	}
	
	
	for (auto& e : mAllRitems)
	{
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform); 

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			
			currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			e->NumFramesDirty--;

		}
	}
}

void MyScene::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));
			matConstants.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void MyScene::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();
	
	//XMMATRIX viewMini = mCameraMini.GetView();
	//XMMATRIX projMini = mCameraMini.GetProj();
	
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	//XMMATRIX viewProjMini = XMMatrixMultiply(viewMini, projMini);
	//XMMATRIX invProjMini = XMMatrixInverse(&XMMatrixDeterminant(projMini), projMini);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	//XMStoreFloat4x4(&mMainPassCB.ViewMini, XMMatrixTranspose(viewMini));
	//XMStoreFloat4x4(&mMainPassCB.ProjMini, XMMatrixTranspose(projMini));
	//XMStoreFloat4x4(&mMainPassCB.ViewProjMini, XMMatrixTranspose(viewProjMini));

	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	//mMainPassCB.EyePosWMini = mCameraMini.GetPosition3f();

	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();

	//간접광 흉내
	mMainPassCB.AmbientLight = { 0.2f, 0.2f, 0.2f, 1.0f };

	int lightCount = 0;
	
	mMainPassCB.Lights[lightCount].Position = { 0,100,0 };
	mMainPassCB.Lights[lightCount].Strength = { 1.0f, 1.0f, 1.0f };
	//mMainPassCB.Lights[lightCount].FalloffStart = 1000;
	//mMainPassCB.Lights[lightCount].FalloffEnd = 5000;

	XMVECTOR lightDir = XMVectorSet(0.05f, sinf(mSunTheta), 0, 0); // -MathHelper::SphericalToCartesian(1.0f, mSunTheta, mSunPhi);
	XMStoreFloat3(&mMainPassCB.Lights[lightCount++].Direction, lightDir);

		mMainPassCB.gFogStart = 100.0f;
		mMainPassCB.gFogRange = 700.0f;
	
	
	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void MyScene::LoadTextures()
{
	auto gridTex = std::make_unique<Texture>();
	gridTex->Name = "gridTex";
	gridTex->Filename = L"Textures/grid2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), gridTex->Filename.c_str(),
		gridTex->Resource, gridTex->UploadHeap));

	auto detailTex = std::make_unique<Texture>();
	detailTex->Name = "detailTex";
	detailTex->Filename = L"Textures/Detail_Texture_7.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), detailTex->Filename.c_str(),
		detailTex->Resource, detailTex->UploadHeap));

	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"Textures/Grass08.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto treeArrayTex = std::make_unique<Texture>();
	treeArrayTex->Name = "treeArrayTex";
	treeArrayTex->Filename = L"Textures/treeArray2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), treeArrayTex->Filename.c_str(),
		treeArrayTex->Resource, treeArrayTex->UploadHeap));

	auto gunshipTex = std::make_unique<Texture>();
	gunshipTex->Name = "gunshipTex";
	gunshipTex->Filename = L"Textures/1K_GunshipTXTR.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), gunshipTex->Filename.c_str(),
		gunshipTex->Resource, gunshipTex->UploadHeap));

	auto missileTex = std::make_unique<Texture>();
	missileTex->Name = "missileTex";
	missileTex->Filename = L"Textures/512K_HellfireTXTR.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), missileTex->Filename.c_str(),
		missileTex->Resource, missileTex->UploadHeap));

	auto flareArrayTex = std::make_unique<Texture>();
	flareArrayTex->Name = "flareArrayTex";
	flareArrayTex->Filename = L"Textures/explosion.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), flareArrayTex->Filename.c_str(),
		flareArrayTex->Resource, flareArrayTex->UploadHeap));

	auto enemyTex = std::make_unique<Texture>();
	enemyTex->Name = "enemyTex";
	enemyTex->Filename = L"Textures/FlyerPlayershipAlbedo.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), enemyTex->Filename.c_str(),
		enemyTex->Resource, enemyTex->UploadHeap));

	auto ememyDetailTex = std::make_unique<Texture>();
	ememyDetailTex->Name = "enemyDetailTex";
	ememyDetailTex->Filename = L"Textures/FlyerPlayershipEmission.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), ememyDetailTex->Filename.c_str(),
		ememyDetailTex->Resource, ememyDetailTex->UploadHeap));

	auto robotTex = std::make_unique<Texture>();
	robotTex->Name = "robotTex";
	robotTex->Filename = L"Textures/monster.dds";//robotBoy.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), robotTex->Filename.c_str(),
		robotTex->Resource, robotTex->UploadHeap));
	
	auto robot_prowlerTex = std::make_unique<Texture>();
	robot_prowlerTex->Name = "robot_prowlerTex";
	robot_prowlerTex->Filename = L"Textures/robot_prowler.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), robot_prowlerTex->Filename.c_str(),
		robot_prowlerTex->Resource, robot_prowlerTex->UploadHeap));

	mTextures[gridTex->Name] = std::move(gridTex);
	mTextures[detailTex->Name] = std::move(detailTex);
	mTextures[grassTex->Name] = std::move(grassTex);
	mTextures[treeArrayTex->Name] = std::move(treeArrayTex);
	mTextures[gunshipTex->Name] = std::move(gunshipTex);
	mTextures[missileTex->Name] = std::move(missileTex);
	mTextures[flareArrayTex->Name] = std::move(flareArrayTex);
	mTextures[enemyTex->Name] = std::move(enemyTex);
	mTextures[ememyDetailTex->Name] = std::move(ememyDetailTex);
	mTextures[robotTex->Name] = std::move(robotTex);
	mTextures[robot_prowlerTex->Name] = std::move(robot_prowlerTex);
}

void MyScene::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable[3];
	texTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	texTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); 
	texTable[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); 
	//texTable[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 3); //본 데이터

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[7];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable[0], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[5].InitAsDescriptorTable(1, &texTable[1], D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[6].InitAsDescriptorTable(1, &texTable[2], D3D12_SHADER_VISIBILITY_PIXEL);
	//slotRootParameter[6].InitAsDescriptorTable(1, &texTable[3], D3D12_SHADER_VISIBILITY_PIXEL);

	slotRootParameter[1].InitAsConstantBufferView(0); //버텍스상수
	slotRootParameter[2].InitAsConstantBufferView(1); //상수
	slotRootParameter[3].InitAsConstantBufferView(2); //매터리얼상수
	slotRootParameter[4].InitAsConstantBufferView(3); //본

	auto staticSamplers = GetStaticSamplers();

	  // 루트 서명은 루트 매개변수들의 배열이다
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(7, slotRootParameter, //루트파라메터 사이즈
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//상수 버퍼 하나로 구성된 서술자 구간을 카리키는 슬롯 하나로 이루어진 루트 서명을 생성한다.
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void MyScene::BuildPostProcessRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstants(12, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mPostProcessRootSignature.GetAddressOf())));
}

void MyScene::BuildPostProcessSobelRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE srvTable0;
	srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable1;
	srvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &srvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable0);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mPostProcessSobelRootSignature.GetAddressOf())));
}

void MyScene::BuildDescriptorHeaps()
{
	int rtvOffset = SwapChainBufferCount;

	const int textureDescriptorCount = 13;
	const int blurDescriptorCount = 4;

	int srvOffset = textureDescriptorCount;
	int sobelSrvOffset = srvOffset + blurDescriptorCount;
	int offscreenSrvOffset = sobelSrvOffset + mSobelFilter->DescriptorCount();

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = textureDescriptorCount + blurDescriptorCount +
		mSobelFilter->DescriptorCount() + 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mCbvSrvUavDescriptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto gridTex = mTextures["gridTex"]->Resource;
	auto detailTex = mTextures["detailTex"]->Resource;
	auto grassTex = mTextures["grassTex"]->Resource;

	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;

	auto gunshipTex = mTextures["gunshipTex"]->Resource;
	auto missileTex = mTextures["missileTex"]->Resource;

	auto flareArrayTex = mTextures["flareArrayTex"]->Resource;

	auto enemyTex = mTextures["enemyTex"]->Resource;
	auto enemyDetailTex = mTextures["enemyDetailTex"]->Resource;
	
	auto robotTex = mTextures["robotTex"]->Resource;
	auto robot_prowlerTex = mTextures["robot_prowlerTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = gridTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = gridTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(gridTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);

	srvDesc.Format = detailTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = detailTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(detailTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);

	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = grassTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);

	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = treeArrayTex->GetDesc().MipLevels;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = gunshipTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = gunshipTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(gunshipTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = missileTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = missileTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(missileTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = flareArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MipLevels = flareArrayTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(flareArrayTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = enemyTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = enemyTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(enemyTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = enemyDetailTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = enemyDetailTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(enemyDetailTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = robotTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = robotTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(robotTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = robot_prowlerTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = robot_prowlerTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(robot_prowlerTex.Get(), &srvDesc, hDescriptor);
	
	// Fill out the heap with the descriptors to the BlurFilter resources.
	
	auto srvCpuStart = mCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto srvGpuStart = mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	mBlurFilter->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, srvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, srvOffset, mCbvSrvUavDescriptorSize),
		mCbvSrvUavDescriptorSize);

	mSobelFilter->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, sobelSrvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, sobelSrvOffset, mCbvSrvUavDescriptorSize),
		mCbvSrvUavDescriptorSize);

	auto rtvCpuStart = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	mOffscreenRT->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, offscreenSrvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, offscreenSrvOffset, mCbvSrvUavDescriptorSize),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvCpuStart, rtvOffset, mRtvDescriptorSize));
}

void MyScene::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skinnedVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", skinnedDefines, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");

	/*mShaders["MiniVS"] = d3dUtil::CompileShader(L"Shaders\\minimap.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["MiniPS"] = d3dUtil::CompileShader(L"Shaders\\minimap.hlsl", defines, "PS", "ps_5_1");*/

	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_0");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mShaders["flareSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\flareSprite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["flareSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\flareSprite.hlsl", nullptr, "GS", "gs_5_0");
	mShaders["flareSpritePS"] = d3dUtil::CompileShader(L"Shaders\\flareSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mShaders["SkyBoxVS"] = d3dUtil::CompileShader(L"Shaders\\SkyBox.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["SkyBoxPS"] = d3dUtil::CompileShader(L"Shaders\\SkyBox.hlsl", nullptr, "PS", "ps_5_0");

	mShaders["horzBlurCS"] = d3dUtil::CompileShader(L"Shaders\\Blur.hlsl", nullptr, "HorzBlurCS", "cs_5_0");
	mShaders["vertBlurCS"] = d3dUtil::CompileShader(L"Shaders\\Blur.hlsl", nullptr, "VertBlurCS", "cs_5_0");

	mShaders["compositeVS"] = d3dUtil::CompileShader(L"Shaders\\Composite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["compositePS"] = d3dUtil::CompileShader(L"Shaders\\Composite.hlsl", nullptr, "PS", "ps_5_0");
	mShaders["sobelCS"] = d3dUtil::CompileShader(L"Shaders\\Sobel.hlsl", nullptr, "SobelCS", "cs_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	mSkinnedInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

}

void MyScene::BuildAnimation(const std::string fileName,std::string clipNmae, float loadScale, bool isMap)
{
	auto dummy = std::make_unique<GameObject>();
	
	dummy->pMesh = playerMesh;
	dummy->boneOffsets = playerboneOffsets;
	dummy->boneIndexToParentIndex = playerboneIndexToParentIndex;
	dummy->boneName = playerboneName;

	dummy->LoadAnimationModel(fileName, loadScale);
	//dummy->LoadGameModel(fileName, loadScale, isMap, true);

	dummy->LoadAnimation(mSkinnedInfo, clipNmae, loadScale);// "AnimStack::Take 001");

	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkinnedModelInst->SkinnedInfo = &mSkinnedInfo;
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo.BoneCount());
	mSkinnedModelInst->ClipName = clipNmae;
	mSkinnedModelInst->TimePos = 0.0f;

	mSkinnedModelInst->SetNowAni("Take001");
}

void MyScene::BuildCollBoxGeometry(Aabb colbox, const std::string geoName, const std::string meshName)
{
	GeometryGenerator geoGen;
	XMFLOAT3 *_box = colbox.GetAabbBox();
	GeometryGenerator::MeshData box = geoGen.CreateBox(_box[1].x, _box[1].y, _box[1].z);

	UINT robotVertexOffset = 0;
	UINT robotIndexOffset = 0;

	SubmeshGeometry robotSubmesh;
	robotSubmesh.IndexCount = (UINT)box.Indices32.size();
	robotSubmesh.StartIndexLocation = robotIndexOffset;
	robotSubmesh.BaseVertexLocation = robotVertexOffset;

	size_t totalVertexCount = 0;
	totalVertexCount += box.Vertices.size();

	UINT k = 0;
	std::vector<Vertex> vertices(totalVertexCount);

	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		auto& p = box.Vertices[i].Position;
		vertices[k].Pos = p;
	}


	std::vector<std::uint32_t> indices;
	indices.insert(indices.end(), std::begin(box.Indices32), std::end(box.Indices32));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs[meshName] = robotSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void MyScene::BuildFbxGeometry(const std::string fileName, const std::string geoName , const std::string meshName, float loadScale , bool isMap, bool hasAniBone)
{
	GeometryGenerator geoGen;
	auto dummy = std::make_unique<GameObject>();
	dummy->LoadGameModel(fileName, loadScale, isMap, hasAniBone);
	
	if (hasAniBone) {
		playerMesh = dummy->pMesh;
		playerboneOffsets = dummy->boneOffsets;
		playerboneIndexToParentIndex = dummy->boneIndexToParentIndex;
		playerboneName = dummy->boneName;
	}
	
	GeometryGenerator::SkinnedMeshData *robot = dummy->GetSkinMeshData();
	int meshSize = dummy->meshSize;

	UINT robotVertexOffset = 0;
	UINT robotIndexOffset = 0;

	SubmeshGeometry robotSubmesh;
	for (int i = 0; i < meshSize; ++i) {
		robotSubmesh.IndexCount += (UINT)robot[i].Indices32.size();
	}
	robotSubmesh.StartIndexLocation = robotIndexOffset;
	robotSubmesh.BaseVertexLocation = robotVertexOffset;

	size_t totalVertexCount = 0;
	for (int i = 0; i < meshSize; ++i) {
		totalVertexCount += robot[i].Vertices.size();
	}

	UINT k = 0;
	std::vector<SkinnedVertex> vertices(totalVertexCount);

	for (int z = 0; z < meshSize; ++z) {

		for (size_t i = 0; i < robot[z].Vertices.size(); ++i, ++k)
		{
			auto& p = robot[z].Vertices[i].Position;
			vertices[k].Pos = p;
			vertices[k].Normal = robot[z].Vertices[i].Normal;
			vertices[k].TexC = robot[z].Vertices[i].TexC;
			vertices[k].BoneWeights = robot[z].Vertices[i].BoneWeights;
			vertices[k].BoneIndices[0] = robot[z].Vertices[i].BoneIndices[0];
			vertices[k].BoneIndices[1] = robot[z].Vertices[i].BoneIndices[1];
			vertices[k].BoneIndices[2] = robot[z].Vertices[i].BoneIndices[2];
			vertices[k].BoneIndices[3] = robot[z].Vertices[i].BoneIndices[3];
		}

	}

	std::vector<std::uint32_t> indices;
	for (int i = 0; i < meshSize; ++i) {
		indices.insert(indices.end(), std::begin(robot[i].Indices32), std::end(robot[i].Indices32));
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SkinnedVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs[meshName] = robotSubmesh;

	mGeometries[geo->Name] = std::move(geo);
	//바운드박스정보도 로드
	mBounds[meshName] = dummy->bounds;
}


void MyScene::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));
	
	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
	
	//
	// 그리드
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gridPsoDesc = opaquePsoDesc;
	gridPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&gridPsoDesc, IID_PPV_ARGS(&mPSOs["grid"])));
	
	//
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	//treeSpritePsoDesc.pRootSignature = mRootSignatureForTrees.Get();
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));

	//
	// PSO for flare sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC flareSpritePsoDesc = opaquePsoDesc;
	flareSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["flareSpriteVS"]->GetBufferPointer()),
		mShaders["flareSpriteVS"]->GetBufferSize()
	};
	flareSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["flareSpriteGS"]->GetBufferPointer()),
		mShaders["flareSpriteGS"]->GetBufferSize()
	};
	flareSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["flareSpritePS"]->GetBufferPointer()),
		mShaders["flareSpritePS"]->GetBufferSize()
	};
	flareSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	flareSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	flareSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&flareSpritePsoDesc, IID_PPV_ARGS(&mPSOs["flareSprites"])));

	//
	// PSO for horizontal blur
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC horzBlurPSO = {};
	horzBlurPSO.pRootSignature = mPostProcessRootSignature.Get();
	horzBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["horzBlurCS"]->GetBufferPointer()),
		mShaders["horzBlurCS"]->GetBufferSize()
	};
	horzBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&horzBlurPSO, IID_PPV_ARGS(&mPSOs["horzBlur"])));

	//
	// PSO for vertical blur
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC vertBlurPSO = {};
	vertBlurPSO.pRootSignature = mPostProcessRootSignature.Get();
	vertBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["vertBlurCS"]->GetBufferPointer()),
		mShaders["vertBlurCS"]->GetBufferSize()
	};
	vertBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&vertBlurPSO, IID_PPV_ARGS(&mPSOs["vertBlur"])));

	//
	// PSO for compositing post process
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC compositePSO = opaquePsoDesc;
	compositePSO.pRootSignature = mPostProcessSobelRootSignature.Get();

	// Disable depth test.
	compositePSO.DepthStencilState.DepthEnable = false;
	compositePSO.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	compositePSO.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	compositePSO.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["compositeVS"]->GetBufferPointer()),
		mShaders["compositeVS"]->GetBufferSize()
	};
	compositePSO.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["compositePS"]->GetBufferPointer()),
		mShaders["compositePS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&compositePSO, IID_PPV_ARGS(&mPSOs["composite"])));

	//
	// PSO for sobel
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC sobelPSO = {};
	sobelPSO.pRootSignature = mPostProcessSobelRootSignature.Get();
	sobelPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["sobelCS"]->GetBufferPointer()),
		mShaders["sobelCS"]->GetBufferSize()
	};
	sobelPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&sobelPSO, IID_PPV_ARGS(&mPSOs["sobel"])));

	//
	// PSO for skinned pass.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
	skinnedOpaquePsoDesc.InputLayout = { mSkinnedInputLayout.data(), (UINT)mSkinnedInputLayout.size() };
	skinnedOpaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["skinnedVS"]->GetBufferPointer()),
		mShaders["skinnedVS"]->GetBufferSize()
	};
	skinnedOpaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skinnedOpaquePsoDesc, IID_PPV_ARGS(&mPSOs["skinnedOpaque"])));
}

void MyScene::BuildFrameResources()
{
	mFrameResources.clear();
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), 1, (UINT)mMaterials.size()));
	
	}
}

void MyScene::BuildMaterials()
{
	int matIndex = 0;
	auto rand = std::make_unique<Material>();
	rand->Name = "rand";
	rand->MatCBIndex = matIndex;
	rand->DiffuseSrvHeapIndex = matIndex++;
	rand->DiffuseAlbedo = XMFLOAT4(1, 1.0f, 1, 1.0f);
	rand->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	rand->Roughness = 0.3f;

	auto gu = std::make_unique<Material>();
	gu->Name = "gu";
	gu->MatCBIndex = matIndex;
	gu->DiffuseSrvHeapIndex = matIndex++;
	gu->DiffuseAlbedo = XMFLOAT4(1, 1, 1, 1);
	gu->FresnelR0 = XMFLOAT3(0.85f, 0.85f, 0.85f);
	gu->Roughness = 0.6f;

	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = matIndex;
	grass->DiffuseSrvHeapIndex = matIndex++;
	grass->DiffuseAlbedo = XMFLOAT4(1, 1, 1, 1);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = matIndex;
	treeSprites->DiffuseSrvHeapIndex = matIndex++;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;

	auto gunship = std::make_unique<Material>();
	gunship->Name = "gunship";
	gunship->MatCBIndex = matIndex;
	gunship->DiffuseSrvHeapIndex = matIndex++;

	auto missile = std::make_unique<Material>();
	missile->Name = "missile";
	missile->MatCBIndex = matIndex;
	missile->DiffuseSrvHeapIndex = matIndex++;
	missile->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	missile->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	missile->Roughness = 0.125f;

	auto flareSprites = std::make_unique<Material>();
	flareSprites->Name = "flareSprites";
	flareSprites->MatCBIndex = matIndex;
	flareSprites->DiffuseSrvHeapIndex = matIndex++;
	flareSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	flareSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	flareSprites->Roughness = 0.125f;

	auto enemy = std::make_unique<Material>();
	enemy->Name = "enemy";
	enemy->MatCBIndex = matIndex;
	enemy->DiffuseSrvHeapIndex = matIndex++;

	auto enemyDetail = std::make_unique<Material>();
	enemyDetail->Name = "enemyDetail";
	enemyDetail->MatCBIndex = matIndex;
	enemyDetail->DiffuseSrvHeapIndex = matIndex++;

	auto robot = std::make_unique<Material>();
	robot->Name = "robot";
	robot->MatCBIndex = matIndex;
	robot->DiffuseSrvHeapIndex = matIndex++;

	auto robot_prowler = std::make_unique<Material>();
	robot_prowler->Name = "robot_prowler";
	robot_prowler->MatCBIndex = matIndex;
	robot_prowler->DiffuseSrvHeapIndex = matIndex++;

	mMaterials["rand"] = std::move(rand);
	mMaterials["gu"] = std::move(gu);
	mMaterials["grass"] = std::move(grass);
	mMaterials["treeSprites"] = std::move(treeSprites);
	mMaterials["gunship"] = std::move(gunship);
	mMaterials["missile"] = std::move(missile);
	mMaterials["flareSprites"] = std::move(flareSprites);
	mMaterials["enemy"] = std::move(enemy);
	mMaterials["enemyDetail"] = std::move(enemyDetail);
	mMaterials["robot"] = std::move(robot);
	mMaterials["robot_prowler"] = std::move(robot_prowler);
}

void MyScene::BuildGameObjects()
{
	int objIndex = 0;

	//objectindex에 맞춰서 그리는 순서 필요

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	auto gridRitem = std::make_unique<GameObject>();

	XMStoreFloat4x4(&gridRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, -50.0f, 0.0f));
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	gridRitem->ObjCBIndex = objIndex++;
	gridRitem->Mat = mMaterials["rand"].get();
	gridRitem->Geo = mGeometries["map00Geo"].get();
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->bounds = mBounds["map00"];
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["map00"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["map00"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["map00"].BaseVertexLocation;

	mOpaqueRitems[(int)RenderLayer::Grid].push_back(gridRitem.get());
	mAllRitems.push_back(std::move(gridRitem));

	///////////////////////////플레이어////////////////////////////////
	//BuildHelicopterGeometry(*m_gunShip.get());

	auto player = std::make_unique<GameObject>();
	XMStoreFloat4x4(&player->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	mPlayerInfo.World = player->World;
	mPlayerInfo.TexTransform = player->TexTransform;

	player->World = Matrix4x4::Multiply(player->m_xmf4x4ToParentTransform, mPlayerInfo.World);
	player->ObjCBIndex = objIndex++;
	player->Mat = mMaterials["robot"].get();
	player->Geo = mGeometries["robot_freeGeo"].get();
	player->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	player->bounds = mBounds["robot_free"];
	player->IndexCount = player->Geo->DrawArgs["robot_free"].IndexCount;
	player->StartIndexLocation = player->Geo->DrawArgs["robot_free"].StartIndexLocation;
	player->BaseVertexLocation = player->Geo->DrawArgs["robot_free"].BaseVertexLocation;

	player->SkinnedCBIndex = 0;
	player->SkinnedModelInst = mSkinnedModelInst.get();

	//플레이어 충돌박스
	BuildCollBoxGeometry(player->bounds, "robot_freeBoxGeo", "robot_freeBox");

	mOpaqueRitems[(int)RenderLayer::Player].push_back(player.get());
	mAllRitems.push_back(std::move(player));

	auto dummy = std::make_unique<GameObject>();
	XMStoreFloat4x4(&dummy->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	dummy->ObjCBIndex = objIndex++;
	dummy->Mat = mMaterials["robot"].get();
	dummy->Geo = mGeometries["robotGeo"].get();
	dummy->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	dummy->bounds = mBounds["robot"];
	dummy->IndexCount = dummy->Geo->DrawArgs["robot"].IndexCount;
	dummy->StartIndexLocation = dummy->Geo->DrawArgs["robot"].StartIndexLocation;
	dummy->BaseVertexLocation = dummy->Geo->DrawArgs["robot"].BaseVertexLocation;

	BuildCollBoxGeometry(dummy->bounds,"dummyBoxGeo","dummyBox");

	mOpaqueRitems[(int)RenderLayer::Enemy].push_back(dummy.get());
	mAllRitems.push_back(std::move(dummy));

	
	//임시라인
	auto lines = std::make_unique<GameObject>();
	XMStoreFloat4x4(&lines->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	lines->ObjCBIndex = objIndex++;
	lines->Mat = mMaterials["robot"].get();
	lines->Geo = mGeometries["robot_freeBoxGeo"].get();
	lines->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	lines->IndexCount = lines->Geo->DrawArgs["robot_freeBox"].IndexCount;
	lines->StartIndexLocation = lines->Geo->DrawArgs["robot_freeBox"].StartIndexLocation;
	lines->BaseVertexLocation = lines->Geo->DrawArgs["robot_freeBox"].BaseVertexLocation;
	mOpaqueRitems[(int)RenderLayer::CollBox].push_back(lines.get());
	mAllRitems.push_back(std::move(lines));


	auto line = std::make_unique<GameObject>();
	XMStoreFloat4x4(&line->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	line->ObjCBIndex = objIndex++;
	line->Mat = mMaterials["robot"].get();
	line->Geo = mGeometries["dummyBoxGeo"].get();
	line->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	line->IndexCount = line->Geo->DrawArgs["dummyBox"].IndexCount;
	line->StartIndexLocation = line->Geo->DrawArgs["dummyBox"].StartIndexLocation;
	line->BaseVertexLocation = line->Geo->DrawArgs["dummyBox"].BaseVertexLocation;
	mOpaqueRitems[(int)RenderLayer::CollBox].push_back(line.get());
	mAllRitems.push_back(std::move(line));


	m_ObjIndex = objIndex;

	////////////////////////////////////////////////////////////////////////////////////////
}

void MyScene::DrawGameObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<GameObject*>& ritems , const int itemState)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
	UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();
	auto skinnedCB = mCurrFrameResource->SkinnedCB->Resource();

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
		auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvUavDescriptorSize);
		
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() +ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() +ri->Mat->MatCBIndex*matCBByteSize;

		if (ri->SkinnedModelInst != nullptr)
		{
			D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex*skinnedCBByteSize;
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(4, skinnedCBAddress);
		}
		else if ((int)RenderLayer::Grid == itemState) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex2(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			tex2.Offset(1, mCbvSrvUavDescriptorSize);

			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootDescriptorTable(5, tex2);
		}
		else if ((int)RenderLayer::Player == itemState ) {    
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex2(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			
			tex2.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvUavDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootDescriptorTable(5, tex2);
		}
		else if ((int)RenderLayer::Enemy == itemState) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex2(mCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

			tex2.Offset(14, mCbvSrvUavDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
			cmdList->SetGraphicsRootDescriptorTable(5, tex2);
		}
		else {
			cmdList->SetGraphicsRootDescriptorTable(0, tex);
			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(5, matCBAddress);
		}

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		
	}
}

void MyScene::DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList)
{
	// Null-out IA stage since we build the vertex off the SV_VertexID in the shader.
	cmdList->IASetVertexBuffers(0, 1, nullptr);
	cmdList->IASetIndexBuffer(nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->DrawInstanced(6, 1, 0, 0);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> MyScene::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0, D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL); 

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}
