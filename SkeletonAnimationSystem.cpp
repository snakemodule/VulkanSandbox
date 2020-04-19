#include "SkeletonAnimationSystem.h"


void SkeletonAnimationSystem::buildHierarchy(const aiNode& jointRoot)
{
	buildHierarchy(jointRoot, &hierarchy);
}

void SkeletonAnimationSystem::buildHierarchy(const aiNode& sourceNode, JointHierarchy* destinationNode)
{
	destinationNode->nodeName = sourceNode.mName.C_Str();	
	destinationNode->children = std::vector<JointHierarchy>(sourceNode.mNumChildren, JointHierarchy(destinationNode));
	for (size_t i = 0; i < sourceNode.mNumChildren; i++)
	{
		buildHierarchy(*sourceNode.mChildren[i], &destinationNode->children[i]);
	}
}

void SkeletonAnimationSystem::getLocalTransform(std::vector<ActiveJointAnimation> animation, aiNodeAnim* pNodeAnim,
	std::vector<glm::mat4>& outLocalT, std::vector<glm::mat4>& outOffsetM)
{
	assert(pNodeAnim->mNumRotationKeys > 0);
	assert(animation.size() == outLocalT.size());
	assert(animation.size() == outOffsetM.size());

	aiQuatKey* rotKeys = pNodeAnim->mRotationKeys;
	int rotCounter = 0;
	aiVectorKey* posKeys = pNodeAnim->mPositionKeys;
	int posCounter = 0;
	for (size_t i = 0; i < animation.size(); i++)
	{
		while (animation[i].time > rotKeys[rotCounter + 1].mTime)
		{
			rotCounter++;
			assert(rotCounter < pNodeAnim->mNumRotationKeys);
		}
		auto & firstRot = rotKeys[rotCounter];
		auto & secondRot = rotKeys[rotCounter + 1];
		float rotDeltaTime = (float)(secondRot.mTime - firstRot.mTime);
		float rotFactor = (animation[i].time - (float)firstRot.mTime) / rotDeltaTime;
		assert(rotFactor >= 0.0f && rotFactor <= 1.0f);
		aiQuaternion RotationQ;
		aiQuaternion::Interpolate(RotationQ, firstRot.mValue, secondRot.mValue, rotFactor);
		RotationQ.Normalize();

		while (animation[i].time > posKeys[posCounter + 1].mTime)
		{
			posCounter++;
			assert(posCounter < pNodeAnim->mNumPositionKeys);
		}
		auto & firstPos = posKeys[posCounter];
		auto & secondPos = posKeys[posCounter + 1];
		float posDeltaTime = (float)(secondPos.mTime - firstPos.mTime);
		float posFactor = (animation[i].time - (float)firstPos.mTime) / posDeltaTime;
		assert(posFactor >= 0.0f && posFactor <= 1.0f);
		const aiVector3D& Start = firstPos.mValue;
		const aiVector3D& End = secondPos.mValue;
		aiVector3D Delta = End - Start;
		aiVector3D Translation = Start + (posFactor * Delta);

		glm::mat4 RotationM = glm::toMat4(glm::quat(RotationQ.w, RotationQ.x, RotationQ.y, RotationQ.z));
		glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(Translation.x, Translation.y, Translation.z));
		outLocalT.push_back(TranslationM * RotationM);
		outOffsetM.push_back(animation[i].offset);
	}
}


/*
void SkeletonAnimationSystem::getInterpolatedRotation(std::vector<ActiveJointAnimation> animation, aiNodeAnim* vNodeAnim,
	std::vector<glm::quat>& Out)
{
	assert(vNodeAnim->mNumRotationKeys > 0);
	assert(animation.size() == Out.size());
	auto numT = animation.size();
	int count = 0;
	for (size_t i = 0; i < numT; i++)
	{
		while (animation[i].time > vNodeAnim->mRotationKeys[count+1].mTime) 
		{
			count++;
			assert(count < vNodeAnim->mNumRotationKeys);
		}
		auto & firstKey = vNodeAnim->mRotationKeys[count];
		auto & secondKey = vNodeAnim->mRotationKeys[count+1];

		float DeltaTime = (float)(secondKey.mTime - firstKey.mTime);
		float Factor = (animation[i].time - (float)firstKey.mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		aiQuaternion::Interpolate(Out[i], firstKey.mValue, secondKey.mValue, Factor);
		Out[i] = Out[i].Normalize();

	}
}

void SkeletonAnimationSystem::getInterpolatedPosition(std::vector<ActiveJointAnimation> animation, 
	aiNodeAnim* vNodeAnim,
	std::vector<glm::vec3> & Out)
{
	assert(vNodeAnim->mNumPositionKeys > 0);
	assert(animation.size() == Out.size());
	size_t numT = animation.size();
	size_t count = 0;
	for (size_t i = 0; i < numT; i++)
	{
		while (animation[i].time > vNodeAnim->mPositionKeys[count + 1].mTime)
		{
			count++;
			assert(count < vNodeAnim->mNumPositionKeys);
		}
		auto & firstKey = vNodeAnim->mPositionKeys[count];
		auto & secondKey = vNodeAnim->mPositionKeys[count + 1];

		float DeltaTime = (float)(secondKey.mTime - firstKey.mTime);
		float Factor = (animation[i].time - (float)firstKey.mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = firstKey.mValue;
		const aiVector3D& End = secondKey.mValue;
		aiVector3D Delta = End - Start;
		Out[i] = Start + Factor * Delta;
	}
}
*/

void SkeletonAnimationSystem::localTransform(std::vector<glm::mat4> translationM,
	std::vector<glm::mat4> rotationM, std::vector<glm::mat4> resultLocalT)
{
	matrixMultiply(translationM, rotationM, resultLocalT);
}

void SkeletonAnimationSystem::globalTranform(std::vector<glm::mat4> & parentGlobalT,
	std::vector<glm::mat4> localT, std::vector<glm::mat4>& resultGlobalT)
{
	matrixMultiply(parentGlobalT, localT, resultGlobalT);
}

void SkeletonAnimationSystem::finalTranform(std::vector<glm::mat4>& globalT,
	std::vector<glm::mat4>& offsetT, std::vector<glm::mat4>& resultFinalT)
{
	matrixMultiply(globalT, offsetT, resultFinalT);
}

//hack bad way to get offset transforms
aiBone* isInMeshBones(aiString name, const aiScene* modelScene) {
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
//hack
aiNodeAnim* boneIsInAnimation(aiString name, const aiScene* modelScene) {
	for (size_t j = 0; j < modelScene->mAnimations[0]->mNumChannels; j++)
	{
		if (name == modelScene->mAnimations[0]->mChannels[j]->mNodeName) {
			return modelScene->mAnimations[0]->mChannels[j];
		}
	}
	return nullptr;
}

//todo move this
glm::mat4 glmMatFromAiMat(aiMatrix4x4 t) {
	glm::mat4 glmmat;
	glmmat[0] = { t.a1, t.b1, t.c1, t.d1 };
	glmmat[1] = { t.a2, t.b2, t.c2, t.d2 };
	glmmat[2] = { t.a3, t.b3, t.c3, t.d3 };
	glmmat[3] = { t.a4, t.b4, t.c4, t.d4 };
	return glmmat;
}

void SkeletonAnimationSystem::addAnimation(int ID, const aiNode & joint, const aiScene* scene)
{
	aiNodeAnim* animationWithBone = boneIsInAnimation(joint.mName, scene);
	if (animationWithBone)
	{
		auto it = hierarchy.activeTracks.find(animationWithBone);
		if (it == hierarchy.activeTracks.end())
		{
			hierarchy.activeTracks.insert({ animationWithBone, {} });
			it = hierarchy.activeTracks.find(animationWithBone);
		}
		it->second.push_back({ ID, 0, glmMatFromAiMat(isInMeshBones(joint.mName, scene)->mOffsetMatrix) });
		++hierarchy.animationCount;
	}

	assert(joint.mNumChildren == hierarchy.children.size());
	for (size_t i = 0; i < joint.mNumChildren; i++)
	{
		addAnimation(ID, *joint.mChildren[i], scene);
	}
}

void SkeletonAnimationSystem::updateHierarchically(JointHierarchy & node) 
{
	node.transformations.localT.clear();
	node.transformations.localT.reserve(node.animationCount);//todo reserve on add/remove anim
	node.transformations.offsetM.clear();
	node.transformations.offsetM.reserve(node.animationCount);
	for (auto it = node.activeTracks.begin(); it != node.activeTracks.end(); it++)
	{
		getLocalTransform(it->second, it->first, node.transformations.localT, node.transformations.offsetM);
	}
	if (node.parent)
	{
		globalTranform(node.parent->transformations.globalT,
			node.transformations.localT, node.transformations.globalT);
	}
	else
	{
		node.transformations.globalT = node.transformations.localT;
	}
	finalTranform(node.transformations.globalT, node.transformations.offsetM, node.transformations.finalT);
	for (size_t i = 0; i < node.children.size(); i++)
	{
		updateHierarchically(node.children[i]);
	}
}

void SkeletonAnimationSystem::update()
{
	updateHierarchically(hierarchy);
	//targetBuffer->

}

void SkeletonAnimationSystem::transformstoVkBuffer(VkBuffer buffer)
{
	//memcpy(ubo.boneTransforms, mymodel->skeleton.finalTransformation.data(), sizeof(glm::mat4)*mymodel->skeleton.finalTransformation.size());
}

//todo make util?
void SkeletonAnimationSystem::matrixMultiply(std::vector<glm::mat4>& matrixA, 
	std::vector<glm::mat4>& matrixB, std::vector<glm::mat4>& matrixAB)
{
	assert(matrixAB.size() == matrixB.size());
	assert(matrixAB.size() == matrixA.size());
	assert(matrixA.size() == matrixB.size());
	size_t count = matrixAB.size();
	glm::mat4* A = matrixA.data();
	glm::mat4* B = matrixB.data();
	glm::mat4* AB = matrixAB.data();
	for (size_t i = 0; i < count; ++i)
	{
		AB[i] = A[i] * B[i];
	}
}

void SkeletonAnimationSystem::matrixMultiply(std::vector<glm::mat4>& matrixA,
	glm::mat4& matrixB, std::vector<glm::mat4>& matrixAB)
{
	assert(matrixAB.size() == matrixA.size());
	size_t count = matrixAB.size();
	glm::mat4* A = matrixA.data();
	glm::mat4& B = matrixB;
	glm::mat4* AB = matrixAB.data();
	for (size_t i = 0; i < count; ++i)
	{
		AB[i] = A[i] * B;
	}
}

SkeletonAnimationSystem::SkeletonAnimationSystem()
{
}


SkeletonAnimationSystem::~SkeletonAnimationSystem()
{
}
