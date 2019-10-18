#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

using std::vector;

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


struct key_event {
	int key;
	int scancode;
	int action;
	int mods;
};

vector<key_event> unconsumed_key_events;

std::vector<DrawableMesh> drawables;


