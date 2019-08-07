#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

struct DrawableMesh {
	int id;
	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;
	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;
	uint32_t IndexCount = 0;
	uint32_t DynamicBufferOffset = 0;
	glm::vec4 AmbientColor;
	glm::vec4 DiffuseColor;
	glm::vec4 SpecularColor;
	float Shininess;
};



std::vector<DrawableMesh> drawables;