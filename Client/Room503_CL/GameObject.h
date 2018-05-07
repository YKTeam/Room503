#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "SkinnedData.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "XMHelper12.h"
#include <tchar.h>
#include <iosfwd>

#include "include\assimp\Importer.hpp"
#include "include\assimp\cimport.h"
#include "include\assimp\postprocess.h"
#include "include\assimp\scene.h"
#pragma comment(lib, "assimp-vc140-mt.lib")

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

////////////////////////////// AABB ///////////////////////////////////

namespace TG {
	struct Triangle
	{
		unsigned short  index[3];
	};
}

class Aabb
{
	//최대 최소값
	DirectX::XMFLOAT3 _max = { 0.01f, 0.01f, 0.01f };
	DirectX::XMFLOAT3 _min = { 0.01f, 0.01f, 0.01f };

	DirectX::XMFLOAT3 _box[8];

public:
	XMFLOAT3* GetAabbBox() { return _box; }
	//맵 콜리전씌우기
	void SetMaxMin(XMFLOAT3 max, XMFLOAT3 min)
	{
		_max = XMFLOAT3(max.x, max.y, max.z);
		_min = XMFLOAT3(min.x, min.y, min.z);
	}
	//최대 최소값 (회전시 갱신?)
	void GetMaxMin(GeometryGenerator::SkinnedMeshData mesh)
	{
		float x = mesh.Vertices[0].Position.x;
		float y = mesh.Vertices[0].Position.y;
		float z = mesh.Vertices[0].Position.z;

		for (int i = 0; i < mesh.Vertices.size(); ++i)
		{
			if (mesh.Vertices[i].Position.x > x)
				x = mesh.Vertices[i].Position.x;
			if (mesh.Vertices[i].Position.y > y)
				y = mesh.Vertices[i].Position.y;
			if (mesh.Vertices[i].Position.z > z)
				z = mesh.Vertices[i].Position.z;
		}
		_max = XMFLOAT3(x, y, z);
		
		x = mesh.Vertices[0].Position.x;
		y = mesh.Vertices[0].Position.y;
		z = mesh.Vertices[0].Position.z;

		for (int i = 0; i < mesh.Vertices.size(); ++i)
		{
			if (mesh.Vertices[i].Position.x < x)
				x = mesh.Vertices[i].Position.x;
			if (mesh.Vertices[i].Position.y < y)
				y = mesh.Vertices[i].Position.y;
			if (mesh.Vertices[i].Position.z < z)
				z = mesh.Vertices[i].Position.z;
		}
		_min = XMFLOAT3(x, y, z) ;
		

		//박스갱신
		_box[0].x = _min.x; _box[0].y = _max.y; _box[0].z = _max.z;
		_box[1].x = _max.x; _box[1].y = _max.y; _box[1].z = _max.z;

		_box[2].x = _min.x; _box[2].y = _max.y; _box[2].z = _min.z;
		_box[3].x = _max.x; _box[3].y = _max.y; _box[3].z = _min.z;

		_box[4].x = _min.x; _box[4].y = _min.y; _box[4].z = _max.z;
		_box[5].x = _max.x; _box[5].y = _min.y; _box[5].z = _max.z;

		_box[6].x = _min.x; _box[6].y = _min.y; _box[6].z = _min.z;
		_box[7].x = _max.x; _box[7].y = _min.y; _box[7].z = _min.z;
	}
	bool IsCollsionAABB( XMFLOAT3 Pos, Aabb *dest , XMFLOAT3 destPos ) {
		if (_min.x + Pos.x > dest->_max.x + destPos.x || _max.x + Pos.x < dest->_min.x + destPos.x) return false;
		if (_min.y + Pos.y > dest->_max.y + destPos.y || _max.y + Pos.y < dest->_min.y + destPos.y) return false;
		if (_min.z + Pos.z > dest->_max.z + destPos.z || _max.z + Pos.z < dest->_min.z + destPos.z) return false;
		return true;
	}
};

struct VertexBoneData
{
	DirectX::XMFLOAT3 BoneWeights;
	BYTE BoneIndices[4];
};

struct SkinnedModelInstance
{
private:
	std::string NowAniName;
	float StartTime = 0;
	float EndTime = 0;

public :
	SkinnedData* SkinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
	std::string ClipName;
	int boneSize = 0;
	float TimePos = 0;

	void SetNowAni(string aniName) {
		NowAniName = aniName;
		//ClipName = aniName;
		if (NowAniName == "idle") {
			StartTime = 0;
			EndTime = 3.00f;//0.05f * 51;// SkinnedInfo->GetClipEndTime(ClipName);//0.05f * 51;
		}
		else if (NowAniName == "walk") {
			StartTime = 3.05f; //0;//3.05f;
			EndTime = 4.55f; //SkinnedInfo->GetClipEndTime(ClipName);//4.55f;
		}
		else if (NowAniName == "die") {
			StartTime = 21.0f;
			EndTime = 23.0f;
		}
	}

	void UpdateSkinnedAnimation(float dt)
	{
		if (TimePos < StartTime) TimePos = StartTime;
		TimePos += dt;

		if (NowAniName == "die" && TimePos > EndTime)
			TimePos = EndTime;
		//루프
		else if (TimePos > EndTime )//SkinnedInfo->GetClipEndTime(ClipName) )
			TimePos = StartTime;

		// Compute the final transforms for this time position.
		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

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
	UINT SkinnedCBIndex = -1;

	// 이 렌더 항목에 연관된 기하구조. 여러 항목이 같은 기하구조를 참조할 수 있음에 주의
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;
	SkinnedModelInstance* SkinnedModelInst = nullptr;
	Aabb bounds;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//BoundingBox Bounds;
	
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

	GeometryGenerator::MeshData *meshData;
	GeometryGenerator::SkinnedMeshData *skinMeshData;

	//본 구조
	std::vector<XMFLOAT4X4> boneOffsets;
	std::vector<pair<string, int>> boneIndexToParentIndex;

	vector<VertexBoneData> mBones;
	vector<pair<std::string,int>> boneName;
	

	UINT numBones = 0;
	UINT numAnimationClips = 0;
	int meshSize = 0;
	const aiScene*                m_pScene;        //모델 정보
	aiMesh* pMesh; //단일메쉬

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	GeometryGenerator::MeshData *GetMeshData() { return meshData; }
	GeometryGenerator::SkinnedMeshData *GetSkinMeshData() { return skinMeshData; }

	//로드 모델
	void LoadGameModel(const string& fileName, float loadScale, bool isMap, bool hasAniBone);
	void LoadAnimationModel(const string& fileName, float loadScale);
	void InitMesh(UINT index, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones , float loadScale);
	void LoadBones(UINT MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones, bool hasAniBone);
	int RobotModelHierarchy(string name);
	void AddBoneData(UINT BoneID, float Weight);
	//탄젠트 축 계산
	void CalculateTangentArray(long vertexCount, SkinnedVertex *data, long triangleCount, TG::Triangle *triangle, XMFLOAT4 *tangent);
	//로드 애니메이션 정보
	void LoadAnimation(SkinnedData& skinInfo, string clipName, float loadScale);
	void ReadBoneOffsets( UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets,float loadScale);
	void ReadBoneHierarchy(UINT numBones, std::vector<pair<string, int>>& boneIndexToParentIndex);
	void ReadAnimationClips( UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations, string clipName , float loadScale);
	void ReadBoneKeyframes( UINT numBones, BoneAnimation& boneAnimation,float loadScale);
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



