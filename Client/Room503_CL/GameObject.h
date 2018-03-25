#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "XMHelper12.h"
#include <tchar.h>
#include <iosfwd>

#include "include\assimp\Importer.hpp"
#include "include\assimp\cimport.h"
#include "include\assimp\postprocess.h"
#include "include\assimp\scene.h"
#pragma comment(lib, "assimp.lib")

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

// 하나의 물체를 그리는 데 필요한 매개변수들을 담는 가벼운 구조체
// 이런 구조체의 구체적인 구성은 응용 프로그램마다 다름
class GameObject
{
public:
	GameObject() = default;
	GameObject(const GameObject& rhs) = delete;
	// 월드 행렬을 기준으로 물체의 국소 공간을 서술하는 세계 행렬
	// 이 행렬은 세계 공간 안에서의 물체의 위치와 방향, 크기를 결정
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	// 
	XMFLOAT4X4	m_xmf4x4ToParentTransform = MathHelper::Identity4x4();
	XMFLOAT4X4	m_xmf4x4ToRootTransform = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는지의 여부를
	// 뜻하는 '덜티' 플래그.   frameresource맏 ㅏ물체의 cbuffer가 있으므로,
	// frameresource마다 갱신을 적용해야 한다. 따라서
	// 물체의 자료를 수정할 때에는 반드시 
	// NumFramesDirty = gNumFrameResources 로 설정해야 한다
	// 그래야 각각의 프레임 자원이 갱신된다
	int NumFramesDirty = gNumFrameResources;

	// 이 렌더 항목의 물체 상수 버퍼에 해당하는 GPU 상수 버퍼의 색인
	UINT ObjCBIndex = -1;

	// 이 렌더 항목에 연관된 기하구조. 여러 항목이 같은 기하구조를 참조할 수 있음에 주의
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	BoundingBox Bounds;
	XMFLOAT3 Dir;

	// DrawIndexedInstanced 매개변수들
	UINT IndexCount = 0;
	UINT InstanceCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	bool isInstance = false;

	virtual void Update(const GameTimer& gt);
	void GravityUpdate(const GameTimer& gt);
	void Pitch(float angle);
	void RotateY(float angle);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 vec3);
	void SetScale(float x, float y, float z);
	XMFLOAT3 GetPosition( );
	XMFLOAT3 GetLook3f();
	XMFLOAT3 GetRight3f();
	XMFLOAT3 GetUp3f();
	void SetScaleWorld3f(XMFLOAT3 f) ;
	void SetLook3f(XMFLOAT3 f) { World._31 = f.x; World._32 = f.y; World._33 = f.z;};
	void SetUp3f(XMFLOAT3 f) { World._21 = f.x; World._22 = f.y; World._23 = f.z; };
	void SetRight3f(XMFLOAT3 f) { World._11 = f.x; World._12 = f.y; World._13 = f.z; };

public:
	TCHAR	m_pstrFrameName[256];
	bool	m_bActive = true;

	GameObject 	*m_pParent = NULL;
	GameObject 	*m_pChild = NULL;
	GameObject 	*m_pSibling = NULL;

	GeometryGenerator::MeshData meshData;
	int meshSize = 0;
	const aiScene*                m_pScene;        //모델 정보
	//GeometryGenerator::MeshData   meshData;        //매쉬 정보
	//UINT                            m_numVertices;
	//UINT                            m_numMaterial;

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	void SetChild(GameObject *pChild);
	GameObject *GetParent() { return(m_pParent); }
	GameObject *FindFrame(_TCHAR *pstrFrameName);
	GeometryGenerator::MeshData GetMeshData() { return meshData; }

	//로드 모델
	void LoadGameModel(const string& fileName, float loadScale);
	void InitMesh(UINT index, const aiMesh * pMesh, float loadScale);
	//계층변환
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL);
	//계층을 파일로부터 읽기
	void LoadFrameHierarchyFromFile(wifstream& InFile, UINT nFrame);
	//지오메트리 정보를 읽기
	void LoadGeometryFromFile(TCHAR *pstrFileName);
	void PrintFrameInfo(GameObject *pGameObject, GameObject *pParent);

};

class PlayerObject : public GameObject
{
	void Update(const GameTimer& gt);
};
class SphereObject : public GameObject
{
	void Update(const GameTimer& gt);
};

class SkyBoxObject : public GameObject
{
	void Update(const GameTimer& gt);
};