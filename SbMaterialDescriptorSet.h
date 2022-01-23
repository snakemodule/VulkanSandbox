#pragma once

#include "vulkan/vulkan.hpp"

#include "SbUniformBuffer.h"

#include "SbDescriptorPool.h"

#include "VulkanInitializers.hpp"


// This class is an attempt to make a more pragmatic version of SbDescriptorSet 
// trying to hide less logic and be more specific with use case.

class SbMaterialDescriptorSet
{
	
public:

	//allocate descriptors for materials
	void allocate(VkDevice device, const SbDescriptorPool& descriptorPool, VkDescriptorSetLayout layout, size_t count, std::vector<VkDescriptorSet>& descriptorSets)
	{
		std::vector<VkDescriptorSetLayout> layouts = std::vector<VkDescriptorSetLayout>(count, layout);
		descriptorSets.resize(count);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool.pool;
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	void writeDescriptor(VkDevice device, uint32_t binding, std::vector<VkDescriptorImageInfo> imageInfo, 
		VkDescriptorType descriptorType, std::vector<VkDescriptorSet>& descriptorSets)
	{
		std::vector<VkWriteDescriptorSet> writes = std::vector<VkWriteDescriptorSet>(descriptorSets.size());
		for (size_t i = 0; i < writes.size(); i++)
		{
			writes[i] = VkWriteDescriptorSet{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				descriptorSets[i],
				binding,
				0,
				1,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				&imageInfo[i]
			};
		}
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);		
	}

};

