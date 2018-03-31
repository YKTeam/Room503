#include "GameObject.h"

void GameObject::Update(const GameTimer& gt)
{

}

void GameObject::SetPosition(float x, float y, float z)
{
	World._41 = x;
	World._42 = y;
	World._43 = z;
}

void GameObject::SetPosition(XMFLOAT3 vec3)
{
	World._41 = vec3.x;
	World._42 = vec3.y;
	World._43 = vec3.z;
}

void GameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParentTransform);
}

XMFLOAT3 GameObject::GetPosition()
{
	return(XMFLOAT3(World._41, World._42, World._43));
}

XMFLOAT3 GameObject::GetLook3f()
{
	return (XMFLOAT3(World._31, World._32, World._33));
}

XMFLOAT3 GameObject::GetRight3f()
{
	return (XMFLOAT3(World._11, World._12, World._13));
}
XMFLOAT3 GameObject::GetUp3f()
{
	return (XMFLOAT3(World._21, World._22, World._23));
}

void GameObject::SetScaleWorld3f(XMFLOAT3 f)
{
	XMMATRIX mtxScale = XMMatrixScaling(f.x, f.y, f.z);
	World = Matrix4x4::Multiply(mtxScale, World);
}

void GameObject::Pitch(float angle)
{
	XMFLOAT3 mRight = XMFLOAT3(World._11, World._12, World._13);
	XMFLOAT3 mUp = XMFLOAT3(World._21, World._22, World._23);
	XMFLOAT3 mLook = XMFLOAT3(World._31, World._32, World._33);

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	World._11 = mRight.x;
	World._12 = mRight.y;
	World._13 = mRight.z;

	World._21 = mUp.x;
	World._22 = mUp.y;
	World._23 = mUp.z;

	World._31 = mLook.x;
	World._32 = mLook.y;
	World._33 = mLook.z;
}

void GameObject::RotateY(float angle)
{
	XMFLOAT3 mRight = XMFLOAT3(World._11, World._12, World._13);
	XMFLOAT3 mUp = XMFLOAT3(World._21, World._22, World._23);
	XMFLOAT3 mLook = XMFLOAT3(World._31, World._32, World._33);

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	World._11 = mRight.x;
	World._12 = mRight.y;
	World._13 = mRight.z;

	World._21 = mUp.x;
	World._22 = mUp.y;
	World._23 = mUp.z;

	World._31 = mLook.x;
	World._32 = mLook.y;
	World._33 = mLook.z;
}

void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void GameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void GameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

//계속늘린다
void GameObject::SetChild(GameObject *pChild)
{
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
	if (pChild) pChild->m_pParent = this;
}

//계층구조에서 찾아낸다
GameObject *GameObject::FindFrame(_TCHAR *pstrFrameName)
{
	GameObject *pFrameObject = NULL;
	if (!_tcsncmp(m_pstrFrameName, pstrFrameName, _tcslen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

//계층을 프린트한다
void GameObject::PrintFrameInfo(GameObject *pGameObject, GameObject *pParent)
{
	TCHAR pstrDebug[128] = { 0 };

	_stprintf_s(pstrDebug, 128, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}


void GameObject::LoadGeometryFromFile(TCHAR *pstrFileName)
{
	wifstream InFile(pstrFileName);
	LoadFrameHierarchyFromFile(InFile, 0);

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[128] = { 0 };
	_stprintf_s(pstrDebug, 128, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	PrintFrameInfo(this, NULL);
#endif
}

void GameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParentTransform, *pxmf4x4Parent) : m_xmf4x4ToParentTransform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&World);
}

void GameObject::LoadFrameHierarchyFromFile(wifstream& InFile, UINT nFrame)
{
}

void GameObject::LoadGameModel(const string& fileName, float loadScale,bool isMap)
{
	if(isMap)
	m_pScene = aiImportFile(fileName.c_str(), aiProcess_JoinIdenticalVertices |        // 동일한 꼭지점 결합, 인덱싱 최적화
		aiProcess_ValidateDataStructure |        // 로더의 출력을 검증
		aiProcess_ImproveCacheLocality |        // 출력 정점의 캐쉬위치를 개선
		aiProcess_RemoveRedundantMaterials |    // 중복된 매터리얼 제거
		aiProcess_GenUVCoords |                    // 구형, 원통형, 상자 및 평면 매핑을 적절한 UV로 변환
		aiProcess_TransformUVCoords |            // UV 변환 처리기 (스케일링, 변환...)
		aiProcess_FindInstances |                // 인스턴스된 매쉬를 검색하여 하나의 마스터에 대한 참조로 제거
		aiProcess_LimitBoneWeights |            // 정점당 뼈의 가중치를 최대 4개로 제한
		aiProcess_OptimizeMeshes |                // 가능한 경우 작은 매쉬를 조인
		aiProcess_GenSmoothNormals |            // 부드러운 노말벡터(법선벡터) 생성
		aiProcess_SplitLargeMeshes |            // 거대한 하나의 매쉬를 하위매쉬들로 분활(나눔)
		aiProcess_Triangulate |                    // 3개 이상의 모서리를 가진 다각형 면을 삼각형으로 만듬(나눔)
		aiProcess_ConvertToLeftHanded |            // D3D의 왼손좌표계로 변환
		aiProcess_PreTransformVertices |       //버텍스미리계산?
		aiProcess_SortByPType);                    // 단일타입의 프리미티브로 구성된 '깨끗한' 매쉬를 만듬
	else
		m_pScene = aiImportFile(fileName.c_str(), aiProcess_JoinIdenticalVertices |        // 동일한 꼭지점 결합, 인덱싱 최적화
			aiProcess_ValidateDataStructure |        // 로더의 출력을 검증
			aiProcess_ImproveCacheLocality |        // 출력 정점의 캐쉬위치를 개선
			aiProcess_RemoveRedundantMaterials |    // 중복된 매터리얼 제거
			aiProcess_GenUVCoords |                    // 구형, 원통형, 상자 및 평면 매핑을 적절한 UV로 변환
			aiProcess_TransformUVCoords |            // UV 변환 처리기 (스케일링, 변환...)
			aiProcess_FindInstances |                // 인스턴스된 매쉬를 검색하여 하나의 마스터에 대한 참조로 제거
			aiProcess_LimitBoneWeights |            // 정점당 뼈의 가중치를 최대 4개로 제한
			aiProcess_OptimizeMeshes |                // 가능한 경우 작은 매쉬를 조인
			aiProcess_GenSmoothNormals |            // 부드러운 노말벡터(법선벡터) 생성
			aiProcess_SplitLargeMeshes |            // 거대한 하나의 매쉬를 하위매쉬들로 분활(나눔)
			aiProcess_Triangulate |                    // 3개 이상의 모서리를 가진 다각형 면을 삼각형으로 만듬(나눔)
			aiProcess_ConvertToLeftHanded |            // D3D의 왼손좌표계로 변환
			aiProcess_SortByPType);
	if (m_pScene) {
		meshSize = m_pScene->mNumMeshes;
		meshData = new GeometryGenerator::MeshData[meshSize];
		//m_numMaterial = m_pScene->mNumMaterials;
		//m_numBones = 0;
		//initScene();

		for (UINT i = 0; i < meshSize; ++i) {

			const aiMesh* pMesh = m_pScene->mMeshes[i];

			InitMesh(i, pMesh , loadScale);

		}

		//m_ModelMeshes.resize(m_meshes.size());
	}
}

void GameObject::InitMesh(UINT index, const aiMesh * pMesh, float loadScale)
{
	meshData[index].Vertices.resize(pMesh->mNumVertices);
	meshData[index].Indices32.resize(pMesh->mNumFaces * 3);

	for (UINT i = 0; i < pMesh->mNumVertices; ++i) {
		XMFLOAT3 pos(&pMesh->mVertices[i].x);
		XMFLOAT3 normal(&pMesh->mNormals[i].x);
		XMFLOAT2 tex;
		if (pMesh->HasTextureCoords(0))
			tex = XMFLOAT2(&pMesh->mTextureCoords[0][i].x);
		else
			tex = XMFLOAT2(0.0f, 0.0f);

		Vertex data;  
		data.Pos.x = pos.x *loadScale;
		data.Pos.y = pos.y *loadScale;
		data.Pos.z = pos.z *loadScale;
		data.Normal.x = normal.x *loadScale;
		data.Normal.y = normal.y *loadScale;
		data.Normal.z = normal.z *loadScale;
		
		data.TexC1.x = tex.x;
		data.TexC1.y = tex.y;
		data.TexC0.x = tex.x;
		data.TexC0.y = tex.y;

		meshData[index].Vertices[i].Position = data.Pos;
		meshData[index].Vertices[i].Normal = data.Normal;
		meshData[index].Vertices[i].TexC = data.TexC0;
		}

	for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		meshData[index].Indices32[i*3] = (face.mIndices[0]);
		meshData[index].Indices32[i*3+1] = (face.mIndices[1]);
		meshData[index].Indices32[i*3+2] = (face.mIndices[2]);
		//m_meshes[index].m_indices.push_back(face.mIndices[0]);
	}
}

void GameObject::GravityUpdate(const GameTimer& gt)
{
	SetPosition(World._41, World._42 - gt.DeltaTime() * 10, World._43);
}

//////////////////////////////////////////////////////////////////////
void SphereObject::Update(const GameTimer& gt)
{
	SetPosition(World._41, World._42 - gt.DeltaTime() * 1, World._43);
}
//////////////////////////////////////////////////////////////////////
