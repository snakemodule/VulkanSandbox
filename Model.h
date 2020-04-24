#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>
#include <string>
#include <assimp/scene.h>

#include "Vertex.h"

#include "Skeleton.h"
#include "SkeletalAnimationComponent.h"

#include "UncompressedAnimation.h"

const int MAXIMUM_BONES = 80;

struct Material {
	glm::vec4 ambientColor;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
};

struct Mesh {
	std::vector<Vertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	Material material;

	Mesh() : vertexBuffer(), indexBuffer() { }

};

struct Model {
	std::vector<Mesh> meshes;
	Skeleton skeleton;
	SkeletalAnimationComponent skeletalAnimationComponent;
};
