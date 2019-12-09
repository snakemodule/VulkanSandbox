#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <map>
#include <string>
#include <array>

#include <glm/glm.hpp>

#include <assimp/scene.h>

const int MAXIMUM_BONES = 80;

struct Vertex;
struct Skeleton;
struct Material;
struct Mesh;
struct Model;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::ivec4 boneIndex = { 0,0,0,0 };
	glm::vec4 animWeight = { 0.0f,0.0f,0.0f,0.0f };

	Vertex(glm::vec3 posVec, glm::vec3 colorvec, glm::vec2 texCoordVec, glm::vec3 normalVec) {
		pos = posVec;
		color = colorvec;
		texCoord = texCoordVec;
		normal = normalVec;
	}

	void AddBoneData(size_t BoneID, float Weight)
	{
		for (int i = 0; i < 4; i++) {
			if (animWeight[i] == 0.0f) {
				boneIndex[i] = BoneID;
				animWeight[i] = Weight;
				return;
			}
		}

		// should never get here - more bones than we have space for
		assert(0);
	}

	static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions() {
		std::array<VkVertexInputBindingDescription, 1> bindingDescription = {};
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	//TODO change vertex info **done?
	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT;
		attributeDescriptions[4].offset = offsetof(Vertex, boneIndex);

		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Vertex, animWeight);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

struct Skeleton {
	std::vector<uint16_t> hierarchy;
	std::vector<aiNodeAnim*> animationChannel; //only stores the keys of one animation

	//maybe put these matrices close in memory? take a look at access pattern
	std::vector<glm::mat4> localTransform; //this isn't even my final form
	std::vector<glm::mat4> offsetMatrix;
	std::vector<glm::mat4> globalTransform;
	std::vector<glm::mat4> inverseBindPose;
	std::vector<glm::mat4> finalTransformation; //behold my final form
};

struct Material {
	glm::vec4 ambientColor;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
};

struct Mesh {
	std::vector<Vertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	Material material;

	Mesh() : vertexBuffer(), indexBuffer() {

	}

};



struct Model {
	std::vector<Mesh> meshes;
	//std::map<std::string, size_t> boneIndexMap; //key=name, value=bone index
	std::map<std::string, size_t> jointIndex;
	Skeleton skeleton;
};





