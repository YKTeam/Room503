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


void GameObject::LoadGameModel(const string& fileName, float loadScale, bool isMap)
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
		skinMeshData = new GeometryGenerator::SkinnedMeshData[meshSize];
		
		//m_numMaterial = m_pScene->mNumMaterials;
		numBones = 0;
		numAnimationClips = m_pScene->mNumAnimations;
		for (UINT i = 0; i < meshSize; ++i) {
			const aiMesh* pMesh = m_pScene->mMeshes[i];

			numBones = pMesh->mNumBones;
			
			mBones.resize(pMesh->mNumVertices);

			LoadBones(i, pMesh, mBones);
			InitMesh(i, pMesh, mBones, loadScale);
		}

		//m_ModelMeshes.resize(m_meshes.size());
	}
}

void GameObject::InitMesh(UINT index, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones, float loadScale)
{
	skinMeshData[index].Vertices.resize(pMesh->mNumVertices);
	skinMeshData[index].Indices32.resize(pMesh->mNumFaces * 3);

	for (UINT i = 0; i < pMesh->mNumVertices; ++i) {
		XMFLOAT3 pos(&pMesh->mVertices[i].x);
		XMFLOAT3 normal(&pMesh->mNormals[i].x);
		XMFLOAT2 tex;
		pMesh->mBones[i]->mWeights;
		if (pMesh->HasTextureCoords(0))
			tex = XMFLOAT2(&pMesh->mTextureCoords[0][i].x);
		else
			tex = XMFLOAT2(0.0f, 0.0f);

		SkinnedVertex data;
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

		data.BoneIndices[0] = Bones[i].BoneIndices[0];
		data.BoneIndices[1] = Bones[i].BoneIndices[1];
		data.BoneIndices[2] = Bones[i].BoneIndices[2];
		data.BoneIndices[3] = Bones[i].BoneIndices[3];

		data.BoneWeights.x = Bones[i].BoneWeights.x;
		data.BoneWeights.y = Bones[i].BoneWeights.y;
		data.BoneWeights.z = Bones[i].BoneWeights.z;

		skinMeshData[index].Vertices[i].Position = data.Pos;
		skinMeshData[index].Vertices[i].Normal = data.Normal;
		skinMeshData[index].Vertices[i].TexC = data.TexC0;
		skinMeshData[index].Vertices[i].BoneIndices[0] = data.BoneIndices[0];
		skinMeshData[index].Vertices[i].BoneIndices[1] = data.BoneIndices[1];
		skinMeshData[index].Vertices[i].BoneIndices[2] = data.BoneIndices[2];
		skinMeshData[index].Vertices[i].BoneIndices[3] = data.BoneIndices[3];
		skinMeshData[index].Vertices[i].BoneWeights = data.BoneWeights;
		}

	for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		skinMeshData[index].Indices32[i*3] = (face.mIndices[0]);
		skinMeshData[index].Indices32[i*3+1] = (face.mIndices[1]);
		skinMeshData[index].Indices32[i*3+2] = (face.mIndices[2]);
	}
}

void GameObject::LoadBones(UINT MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	for (UINT i = 0; i < pMesh->mNumBones; i++) {
		UINT BoneIndex = 0;
		string BoneName(pMesh->mBones[i]->mName.data);
		pair<string, int> boneNameIndex = make_pair(BoneName, i);

		boneName.push_back(boneNameIndex);

		for (UINT j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
			UINT VertexID =  pMesh->mBones[i]->mWeights[j].mVertexId;//m_Entries[MeshIndex].BaseVertex +
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;

			for (int z = 0; z < 4; z++) {
				//
				if (Bones[VertexID].BoneWeights.x == 0.0) {
					Bones[VertexID].BoneIndices[z] = i;
					Bones[VertexID].BoneWeights.x = Weight;
					break;
				}
				else if (Bones[VertexID].BoneWeights.y == 0.0)
				{
					Bones[VertexID].BoneIndices[z] = i;
					Bones[VertexID].BoneWeights.y = Weight;
					break;
				}
				else if (Bones[VertexID].BoneWeights.z == 0.0)
				{
					Bones[VertexID].BoneIndices[z] = i;
					Bones[VertexID].BoneWeights.z = Weight;
					break;
				}
				else Bones[VertexID].BoneIndices[z] = i;
			}
		}
	}
}

void GameObject::LoadAnimation(SkinnedData& skinInfo , string clipName)
{
	if (m_pScene) {
		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<pair<string,int>> boneIndexToParentIndex;
		std::unordered_map<std::string, AnimationClip> animations;

		//애니메이션 정보 받아오기
		if (m_pScene->HasAnimations())
		{
			ReadBoneOffsets(numBones, boneOffsets);
			ReadBoneHierarchy(numBones, boneIndexToParentIndex);
			ReadAnimationClips(numBones, numAnimationClips, animations, clipName);
			skinInfo.Set(boneIndexToParentIndex, boneOffsets, animations);
		}
	}
}

void GameObject::ReadBoneOffsets(UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets)
{
	boneOffsets.resize(numBones);

	for (UINT i = 0; i < meshSize; ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];

		//본 정보 받기
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const aiBone* pBone = pMesh->mBones[j];

				boneOffsets[j](0,0) = pBone->mOffsetMatrix.a1;
				boneOffsets[j](0, 1) = pBone->mOffsetMatrix.a2;
				boneOffsets[j](0, 2) = pBone->mOffsetMatrix.a3;
				boneOffsets[j](0, 3) = pBone->mOffsetMatrix.a4;

				boneOffsets[j](1, 0) = pBone->mOffsetMatrix.c1;
				boneOffsets[j](1, 1) = pBone->mOffsetMatrix.c2;
				boneOffsets[j](1, 2) = pBone->mOffsetMatrix.c3;
				boneOffsets[j](1, 3) = pBone->mOffsetMatrix.c4;

				boneOffsets[j](2, 0) = pBone->mOffsetMatrix.b1;
				boneOffsets[j](2, 1) = pBone->mOffsetMatrix.b2;
				boneOffsets[j](2, 2) = pBone->mOffsetMatrix.b3;
				boneOffsets[j](2, 3) = pBone->mOffsetMatrix.b4;

				boneOffsets[j](3, 0) = pBone->mOffsetMatrix.d1;
				boneOffsets[j](3, 1) = pBone->mOffsetMatrix.d2;
				boneOffsets[j](3, 2) = pBone->mOffsetMatrix.d3;
				boneOffsets[j](3, 3) = pBone->mOffsetMatrix.d4;
			}
		}
	}
}
void GameObject::ReadBoneHierarchy(UINT numBones, std::vector<pair<string,int>>& boneIndexToParentIndex)
{
	boneIndexToParentIndex.resize(numBones);

	for (UINT i = 0; i < meshSize; ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];

		//본 정보 받기
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const char *bonesname;
				const char *parentName;
				bool isCheck = false;
				int myindex = 0;
				const aiBone* pBone = pMesh->mBones[j];

				bonesname = boneName[j].first.c_str();
				
				//본갯수만큼 본의 이름데이터에 이름없으면 
				parentName = m_pScene->mRootNode->FindNode(bonesname)->mParent->mName.C_Str();
				for (int y = 0; ; ++y) {
					
					for (int z = 0; z < numBones; ++z) {
						if (boneName[z].first == parentName) {
							boneIndexToParentIndex[j].first = parentName;
							isCheck = true;
							break;
						}
					}
					if (isCheck) break;
					if (m_pScene->mRootNode->FindNode(parentName)->mParent == nullptr ||
						strcmp( parentName,"RootNode") == 0 ) break;
					parentName = m_pScene->mRootNode->FindNode(parentName)->mParent->mName.C_Str();
				}
				//본에서 이름을 찾아서 인덱스를 넘겨준다
				for (auto vet = boneName.begin(); vet != boneName.end(); ++vet)
				{
					if ((*vet).first == parentName) {
						myindex = (*vet).second;
						boneIndexToParentIndex[j].second = myindex; //부모이름인 본을 찾아서
					}
					else if (strcmp(parentName, "RootNode") == 0)
					{
						boneIndexToParentIndex[j].first = "RootNode";
						boneIndexToParentIndex[j].second = -1;
					}
				}
				
				//본 갯수만큼 리사이즈 한 뒤, 루트부터 돌아가면서 번호를?매겨? 시발?모라
			}
		}
	}
}
void GameObject::ReadAnimationClips(UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations, string clipName)
{
	for (UINT i = 0; i < meshSize; ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];

		//본 정보 받기
		if (pMesh->HasBones())
		{
			for (UINT j = 0; j < numBones; ++j) {
				const aiBone* pBone = pMesh->mBones[j];

				for (UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
				{
					AnimationClip clip;
					clip.BoneAnimations.resize(numBones);

					for (UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
					{
						ReadBoneKeyframes(boneIndex, clip.BoneAnimations[boneIndex]);
					}

					animations[clipName] = clip;
				}


			}
		}
	}
}

void GameObject::ReadBoneKeyframes( UINT numBones, BoneAnimation& boneAnimation)
{
	for (int i = 0; i < m_pScene->mNumAnimations; i++)
	{
		aiAnimation *animation = m_pScene->mAnimations[i];
		boneAnimation.Keyframes.resize(animation->mChannels[0]->mNumPositionKeys);
		float time = 0;
		for (int j = 0; j < animation->mChannels[0]->mNumPositionKeys; j++)
		{
			aiNodeAnim *nodeAnimationPos = animation->mChannels[(numBones*3)];
			aiNodeAnim *nodeAnimationRot = animation->mChannels[(numBones*3)+1];
			aiNodeAnim *nodeAnimationScl = animation->mChannels[(numBones*3)+2];
			
			XMFLOAT3 position;
			XMFLOAT3 scale;
			XMFLOAT4 rotation;

			/* load all keyframes */
			time += animation->mTicksPerSecond / animation->mDuration;
			{
				position.x = nodeAnimationPos->mPositionKeys[j].mValue.x;
				position.y = nodeAnimationPos->mPositionKeys[j].mValue.y;
				position.z = nodeAnimationPos->mPositionKeys[j].mValue.z;

				scale.x = nodeAnimationScl->mScalingKeys[j].mValue.x;
				scale.y = nodeAnimationScl->mScalingKeys[j].mValue.y;
				scale.z = nodeAnimationScl->mScalingKeys[j].mValue.z;

				rotation.x = nodeAnimationRot->mRotationKeys[j].mValue.x;
				rotation.y = nodeAnimationRot->mRotationKeys[j].mValue.y;
				rotation.z = nodeAnimationRot->mRotationKeys[j].mValue.z;
				rotation.w = nodeAnimationRot->mRotationKeys[j].mValue.w;

				boneAnimation.Keyframes[j].TimePos = time;
				boneAnimation.Keyframes[j].Translation = position;
				boneAnimation.Keyframes[j].Scale = scale;
				boneAnimation.Keyframes[j].RotationQuat = rotation;
			}
		}
	}


	/*UINT numKeyframes = 0;

	const aiAnimation* pAni = m_pScene->mAnimations[0];

	numKeyframes = pAni->mNumChannels;

	boneAnimation.Keyframes.resize(numKeyframes);

	for (UINT i = 0; i < numKeyframes; ++i)
	{
		float t = 0.0f;
		XMFLOAT3 p(0.0f, 0.0f, 0.0f);
		XMFLOAT3 s(1.0f, 1.0f, 1.0f);
		XMFLOAT4 q(0.0f, 0.0f, 0.0f, 1.0f);
		
		t = pAni->mChannels[i]->mPositionKeys->mTime;
		p.x = pAni->mChannels[i]->mPositionKeys->mValue.x;
		p.y = pAni->mChannels[i]->mPositionKeys->mValue.y;
		p.z = pAni->mChannels[i]->mPositionKeys->mValue.z;

		s.x = pAni->mChannels[i]->mScalingKeys->mValue.x;
		s.y = pAni->mChannels[i]->mScalingKeys->mValue.y;
		s.z = pAni->mChannels[i]->mScalingKeys->mValue.z;

		q.x = pAni->mChannels[i]->mRotationKeys->mValue.x;
		q.y = pAni->mChannels[i]->mRotationKeys->mValue.y;
		q.z = pAni->mChannels[i]->mRotationKeys->mValue.z;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = p;
		boneAnimation.Keyframes[i].Scale = s;
		boneAnimation.Keyframes[i].RotationQuat = q;
	}
*/
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
