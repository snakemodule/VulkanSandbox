#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <map>

#include "vulkan/vulkan.h"

#include <assimp/scene.h>


#include "SbUniformBuffer.h"



class SkeletonAnimationSystem
{
public:	

	struct ActiveJointAnimation {
		int id;
		float time;
		glm::mat4 offset;
	};

	struct JointHierarchy {
		std::string nodeName;
		struct AnimationTransformSOA {
			std::vector<glm::mat4> localT;
			std::vector<glm::mat4> offsetM;
			std::vector<glm::mat4> globalT;
			std::vector<glm::mat4> finalT;
		} transformations;
		int animationCount = 0; //move out? single value for entire hierarchy?
		std::map<aiNodeAnim*, std::vector<ActiveJointAnimation>> activeTracks;
		JointHierarchy* parent = nullptr;
		std::vector<JointHierarchy> children = {};

		JointHierarchy(JointHierarchy* parent) {
			this->parent = parent;
		}

		JointHierarchy() {}

	} hierarchy; 

	//SbUniformBuffer<glm::mat4> & targetBuffer;

	void buildHierarchy(const aiNode& jointRoot);
	void buildHierarchy(const aiNode& sourceNode, JointHierarchy * destinationNode);

	void getLocalTransform(std::vector<ActiveJointAnimation> animation, 
		aiNodeAnim * pNodeAnim, std::vector<glm::mat4>& outLocalT, std::vector<glm::mat4>& outOffsetM);

	void getInterpolatedRotation(std::vector<ActiveJointAnimation> animation, 
		aiNodeAnim * vNodeAnim,	std::vector<glm::quat> & Out);

	void getInterpolatedPosition(std::vector<ActiveJointAnimation> animation, 
		aiNodeAnim * vNodeAnim, std::vector<glm::vec3> & Out);


	void localTransform(std::vector<glm::mat4> translationM, 
		std::vector<glm::mat4> rotationM, std::vector<glm::mat4> resultLocalT);
	void globalTranform(std::vector<glm::mat4>& parentGlobalT, 
		std::vector<glm::mat4> localT_offsetM, std::vector<glm::mat4>& resultGlobalT);
	void finalTranform(std::vector<glm::mat4>& globalT,	std::vector<glm::mat4>& offset, std::vector<glm::mat4>& resultFinalT);
	
	void addAnimation(int ID, const aiNode& jointRoot, const aiScene* scene);
	void removeSkeleton();
	void fillTransformatonBuffer();

	void updateHierarchically(JointHierarchy & destinationNode);

	void update();

	void transformstoVkBuffer(VkBuffer buffer);

	void matrixMultiply(std::vector<glm::mat4>& matrixA, std::vector<glm::mat4>& matrixB, std::vector<glm::mat4>& matrixAB);

	void matrixMultiply(std::vector<glm::mat4>& matrixA, glm::mat4 & matrixB, std::vector<glm::mat4>& matrixAB);

	SkeletonAnimationSystem();
	~SkeletonAnimationSystem();
};

