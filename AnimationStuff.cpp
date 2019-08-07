#include "AnimationStuff.h"

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

unsigned int AnimationStuff::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

unsigned int AnimationStuff::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

unsigned int AnimationStuff::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

void AnimationStuff::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void AnimationStuff::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void AnimationStuff::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

aiBone* AnimationStuff::isInMeshBones(aiString name, const aiScene* modelScene) {
	for (unsigned int i = 0; i < modelScene->mNumMeshes; i++) {
		for (size_t j = 0; j < modelScene->mMeshes[i]->mNumBones; j++)
		{
			//std::cout << "searching for root..." << std::endl;
			if (name == modelScene->mMeshes[i]->mBones[j]->mName) {
				return modelScene->mMeshes[i]->mBones[j];
			}
		}
	}
	return nullptr;
}

glm::mat4 AnimationStuff::makeAnimationMatrix(aiNodeAnim* channel, float AnimationTime) {

	// Interpolate scaling and generate scaling transformation matrix
	aiVector3D Scaling;
	CalcInterpolatedScaling(Scaling, AnimationTime, channel);
	glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(Scaling.x, Scaling.y, Scaling.z));

	// Interpolate rotation and generate rotation transformation matrix
	aiQuaternion RotationQ;
	CalcInterpolatedRotation(RotationQ, AnimationTime, channel);
	glm::mat4 RotationM = glm::toMat4(glm::quat(RotationQ.w, RotationQ.x, RotationQ.y, RotationQ.z));

	// Interpolate translation and generate translation transformation matrix
	aiVector3D Translation;
	CalcInterpolatedPosition(Translation, AnimationTime, channel);
	glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(Translation.x, Translation.y, Translation.z));

	// Combine the above transformations
	return TranslationM * RotationM;// *ScalingM;
}

aiNode* AnimationStuff::findRootJoint(aiNode* node, const aiScene* scene) {

	if (isInMeshBones(node->mName, scene))
	{
		if (node->mParent == nullptr || !isInMeshBones(node->mParent->mName, scene))
		{
			return node;
		}
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		auto result = findRootJoint(node->mChildren[i], scene);
		if (result) {
			return result;
		}
	}
	return nullptr;
}

aiNodeAnim* AnimationStuff::boneIsInAnimation(aiString name, const aiScene* modelScene) {
	for (size_t j = 0; j < modelScene->mAnimations[0]->mNumChannels; j++)
	{
		if (name == modelScene->mAnimations[0]->mChannels[j]->mNodeName) {
			return modelScene->mAnimations[0]->mChannels[j];
		}
	}
	return nullptr;
}

int AnimationStuff::flattenJointHierarchy(aiNode* node, Model& model, int insertIndex, int parentIndex, const aiScene* scene)
{
	int createdEntry = 0;
	aiBone* boneExists = isInMeshBones(node->mName, scene);
	if (boneExists)
	{
		model.skeleton.hierarchy.emplace_back(parentIndex);

		aiNodeAnim* animChannelOfBone = boneIsInAnimation(node->mName, scene);
		if (animChannelOfBone) {
			model.skeleton.animationChannel.emplace_back(animChannelOfBone);
		}

		model.skeleton.localTransform.emplace_back(glmMatFromAiMat(node->mTransformation));

		model.skeleton.offsetMatrix.emplace_back(glmMatFromAiMat(boneExists->mOffsetMatrix));

		aiQuaternion q;
		aiVector3D v;
		boneExists->mOffsetMatrix.DecomposeNoScaling(q, v);

		model.jointIndex.insert(std::pair<std::string, size_t>(node->mName.C_Str(), insertIndex)); //plan: use this map to set boneindex of vertex
		createdEntry = 1;
		parentIndex = insertIndex;
	}


	// then do the same for each of its children
	int childCounter = 0;
	for (unsigned int i = 0; (i < node->mNumChildren); i++)
	{
		childCounter += flattenJointHierarchy(node->mChildren[i], model, insertIndex + createdEntry + childCounter, parentIndex, scene);
	}

	return childCounter + createdEntry; //return number of entries created
}

//put in utils?
glm::mat4 AnimationStuff::glmMatFromAiMat(aiMatrix4x4 t) {
	glm::mat4 glmmat;
	glmmat[0] = { t.a1, t.b1, t.c1, t.d1 };
	glmmat[1] = { t.a2, t.b2, t.c2, t.d2 };
	glmmat[2] = { t.a3, t.b3, t.c3, t.d3 };
	glmmat[3] = { t.a4, t.b4, t.c4, t.d4 };
	return glmmat;
}

void AnimationStuff::prepareInverseBindPose(Skeleton& s)
{
	s.hierarchy.shrink_to_fit();
	s.animationChannel.shrink_to_fit();
	s.localTransform.shrink_to_fit();
	s.offsetMatrix.shrink_to_fit();
	s.globalTransform.resize(s.hierarchy.size());
	s.inverseBindPose.resize(s.hierarchy.size());
	s.finalTransformation.resize(s.hierarchy.size());


	// the root has no parent
	s.globalTransform[0] = s.localTransform[0];
	s.inverseBindPose[0] = glm::inverse(s.globalTransform[0]);


	for (unsigned int i = 1; i < s.hierarchy.size(); ++i)
	{
		const uint16_t parentJoint = s.hierarchy[i];
		s.globalTransform[i] = s.globalTransform[parentJoint] * s.localTransform[i];
		s.inverseBindPose[i] = glm::inverse(s.globalTransform[i]);
	}
}


AnimationStuff::AnimationStuff()
{
}


AnimationStuff::~AnimationStuff()
{
}
