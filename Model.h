#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>
#include <string>
#include <assimp/scene.h>

#include "AnimatedVertex.h"

#include "Skeleton.h"
#include "SkeletalAnimationComponent.h"


const int MAXIMUM_BONES = 80;

struct Material {
	glm::vec4 ambientColor;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	//array of texures?
	//diffuse(color)
	//normal(bump)
	//displacement(height)
	//specularity(reflection)
};

struct MeshData
{
	std::vector<AnimatedVertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;
};

struct AnimatedMesh {
	std::vector<AnimatedVertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	Material material;

	AnimatedMesh() : vertexBuffer(), indexBuffer() { }

};

struct StaticMesh
{
	std::vector<AnimatedVertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	//Material material;

	StaticMesh() : vertexBuffer(), indexBuffer() 
	{

	}

};

struct AnimatedModel {
	std::vector<AnimatedMesh> meshes;
	Skeleton skeleton;
	SkeletalAnimationComponent skeletalAnimationComponent;

	AnimatedModel(std::string modelFile);


};


