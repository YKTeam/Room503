#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include "d3dUtil.h"
#include "MathHelper.h"


struct Subset
{
	UINT Id = -1;
	UINT VertexStart = 0;
	UINT VertexCount = 0;
	UINT FaceStart = 0;
	UINT FaceCount = 0;
};
///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	Keyframe(float time, DirectX::XMFLOAT3 tr, DirectX::XMFLOAT3 sc, DirectX::XMFLOAT4 rt) {
		TimePos = time;
		Translation = tr;
		Scale = sc;
		RotationQuat = rt;
	};
	~Keyframe();

    float TimePos;
	DirectX::XMFLOAT3 Translation;
    DirectX::XMFLOAT3 Scale;
    DirectX::XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, DirectX::XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes; 	
};

///<summary>
/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
/// An AnimationClip requires a BoneAnimation for every bone to form
/// the animation clip.    
///</summary>
struct AnimationClip
{
	float GetClipStartTime()const; // 이 클립 뼈대중 가장 이른 시작시간
	float GetClipEndTime()const;

	//각 본 애니메이션을 훑으며 애니메이션 보간
    void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;

	//뼈대별 애니메이션들
    std::vector<BoneAnimation> BoneAnimations; 	
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::vector<std::pair<std::string, int>>& boneHierarchy,
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations);

	 // 
    void GetFinalTransforms(const std::string& clipName, float timePos, 
		 std::vector<DirectX::XMFLOAT4X4>& finalTransforms)const;

private:
    // 뼈대 부모의 색인
	std::vector<std::pair<std::string,int>> mBoneHierarchy;

	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;
   
	std::unordered_map<std::string, AnimationClip> mAnimations;
};
 
#endif // SKINNEDDATA_H