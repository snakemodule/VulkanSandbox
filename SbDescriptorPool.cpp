#include "SbDescriptorPool.h"

#include <utility>

#include "VulkanInitializers.hpp"

SbDescriptorPool::SbDescriptorPool(VkDevice device)
	: device(device)
{
}

SbDescriptorPool::~SbDescriptorPool()
{
}

void SbDescriptorPool::createDescriptorPool(std::vector<VkDescriptorPoolSize> poolSize, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	poolInfo.pPoolSizes = poolSize.data();
	poolInfo.maxSets = maxSets;
	
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

std::vector<VkDescriptorSet> SbDescriptorPool::allocateDescriptorSet(const uint32_t n, const VkDescriptorSetLayout dsl)
{
	std::vector<VkDescriptorSetLayout> layouts(n, dsl);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();// &attachmentWriteSubpass.DS_Layout;

	std::vector<VkDescriptorSet> result(n);
	auto vkresult = vkAllocateDescriptorSets(device, &allocInfo, result.data());
	if (vkresult != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	return result;
}


void SbDescriptorPool::destroy()
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}
