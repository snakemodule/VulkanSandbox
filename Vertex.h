#pragma once

#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	
	Vertex() 
	{

	}

	Vertex(glm::vec3 posVec, glm::vec3 colorvec, glm::vec2 texCoordVec, glm::vec3 normalVec) {
		pos = posVec;
		color = colorvec;
		texCoord = texCoordVec;
		normal = normalVec;
	}
		
	static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions() {
		std::array<VkVertexInputBindingDescription, 1> bindingDescription = {};
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

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
		

		return attributeDescriptions;
	}

	static VkPipelineVertexInputStateCreateInfo getInputStateCI() {
		auto bind = getBindingDescriptions();
		auto attr = getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputStateCI;
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(bind.size());
		vertexInputStateCI.pVertexBindingDescriptions = bind.data();
		vertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
		vertexInputStateCI.pVertexAttributeDescriptions = attr.data();
		return vertexInputStateCI;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};