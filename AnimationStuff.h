#pragma once

#include <assimp/scene.h> // Output data structure
#include <glm/glm.hpp>

#include "Model.h"

/*
This class should be functions in a namespace instead of class.
*/
class AnimationStuff
{

	static unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

	static unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);

	static unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

	static void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	static void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	static void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	static aiBone* isInMeshBones(aiString name, const aiScene* scene);

	static aiNodeAnim* boneIsInAnimation(aiString name, const aiScene* scene);

	static glm::mat4 glmMatFromAiMat(aiMatrix4x4 t);

public:

	static glm::mat4 makeAnimationMatrix(aiNodeAnim* channel, float AnimationTime);

	static aiNode* findRootJoint(aiNode* node, const aiScene* scene);

	static int flattenJointHierarchy(aiNode* node, Model& model, int insertIndex, int parentIndex, const aiScene*);



	AnimationStuff();
	~AnimationStuff();
};

