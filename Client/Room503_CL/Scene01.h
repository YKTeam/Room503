#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "GameObject.h"
#include "BlurFilter.h"
#include "SobelFilter.h"
#include "RenderTarget.h"
#include "ShadowMap.h"
#include <tchar.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")



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
	void UpdateSkinnedCBs(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateShadowTransform(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateShadowPassCB(const GameTimer& gt);

	void MenuSceneKeyboardInput(const GameTimer& gt);
	void MenuSceneUpdate(const GameTimer& gt);
	void MenuSceneRender(const GameTimer& gt);

	void InitGameScene();
	void GameSceneKeyboardInput(const GameTimer& gt);
	void GameSceneUpdate(const GameTimer& gt);
	void GameSceneRender(const GameTimer& gt);

	//void BuildConstantBuffers();
	void LoadTextures();
	void BuildRootSignature();
	void BuildPostProcessRootSignature();
	void BuildPostProcessSobelRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();

	void BuildShapeGeometry();
	void BuildCollBoxGeometry(Aabb colbox, const std::string geoName, const std::string meshName, bool isTile);
	void BuildFbxGeometry(const std::string fileName, const std::string geoName, const std::string meshName, float loadScale, bool isMap, bool hasAniBone); //본갯수리턴
	void BuildAnimation(const std::string fileName, const std::string clipName, float loadScale, bool isMap);

	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildGameObjects();
	void DrawGameObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<GameObject*>& ritems, const int itemState);
	void DrawSceneToShadowMap();
	void DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;
	UINT mCbvSrvUavDescriptorSize = 0;
	UINT mCbvSrvDescriptor2Size = 0;
	UINT mSkyTexHeapIndex = 0;
	UINT mShadowMapHeapIndex = 0;
	//그림자용 빈 소스
	UINT mNullCubeSrvIndex = 0;
	UINT mNullTexSrvIndex = 0;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

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

	// List of all the render items.
	std::vector<std::unique_ptr<GameObject>> mAllRitems;
	std::vector<GameObject*> mOpaqueRitems[(int)RenderLayer::Count];
	std::unique_ptr<BlurFilter> mBlurFilter;
	std::unique_ptr<SobelFilter> mSobelFilter = nullptr;
	std::unique_ptr<RenderTarget> mOffscreenRT = nullptr;

	bool mFrustumCullingEnabled = true;
	BoundingFrustum mCamFrustum;

	PassConstants mMainPassCB; //index 0 pass
	PassConstants mShadowPassCB; //index 1 pass

								 //쉐도우매핑
	std::unique_ptr<ShadowMap> mShadowMap;
	DirectX::BoundingSphere mSceneBounds;

	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;
	XMFLOAT3 mLightPosW;
	XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
	XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

	float mLightRotationAngle = 0.0f;
	XMFLOAT3 mBaseLightDirections[3] = {
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	XMFLOAT3 mRotatedLightDirections[3];

	//스키닝 애니메이션데이터 
	std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
	std::unique_ptr<SkinnedModelInstance> mSkinnedFriendModelInst;
	SkinnedData mSkinnedInfo;
	Subset mSkinnedSubsets;
	//메쉬랑 본오프셋이랑 
	aiMesh* playerMesh;
	std::vector<XMFLOAT4X4> playerboneOffsets;
	std::vector<pair<string, int>> playerboneIndexToParentIndex;
	vector<pair<std::string, int>> playerboneName;

	bool mIsWireframe = false;
	float mSunTheta = (float)205.917;

	//게임(플레이어) 에너지
	float mEnergy = 0.5f;
	//블러 레벨 0~ 2
	int blurLevel = 0;
	float startDeathTime = 0.0f;
	//현재 씬 상태 (에 따라 업데이트, 입력, 렌더 [모든 데이터는 한번에 로딩])
	int nowScene = (int)Scene::Menu;
	//메뉴관련 
	bool startMouseOnVal = false;
	bool quitMouseOnVal = false;
	//구조가 생성자로 생성되는구조임
	int m_ObjIndex = 0;
	int m_CameraMoveLevel = 0;
	float mx = 0, my = 0;
	
	Camera mCamera;
	POINT mLastMousePos;
};