#pragma once

#include <vector>

#include "vulkan/vulkan.h"

class SbDescriptorPool
{
	VkDevice device;


public:
	VkDescriptorPool pool;


	SbDescriptorPool(VkDevice);
	~SbDescriptorPool();

	void createDescriptorPool(std::vector<VkDescriptorPoolSize> poolSize, uint32_t maxSets);


	std::vector<VkDescriptorSet> allocateDescriptorSet(const uint32_t n, const VkDescriptorSetLayout dsl);

	

	void destroy();
	
};

