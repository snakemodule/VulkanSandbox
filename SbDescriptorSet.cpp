#include "SbDescriptorSet.h"

#include "VulkanInitializers.hpp"
#include "SbDescriptorPool.h"


void SbDescriptorSet::updateDescriptors()
{
	//for each descriptor set instance
	for (size_t setInstance = 0; setInstance < allocatedDSs.size(); setInstance++)
	{
		std::vector<VkDescriptorBufferInfo> vkBufferInfos(bufInfo.size());
		std::vector<VkDescriptorImageInfo> vkImageInfos(imgInfo.size());
		std::vector<VkWriteDescriptorSet> descriptorWrites(imgInfo.size() + bufInfo.size());

		//convert our binding infos to vk infos
		for (size_t i = 0; i < imgInfo.size(); i++) 
		{
			VkDescriptorImageInfo vkInfo = {};
			vkInfo.imageLayout = imgInfo[i].layout;
			vkInfo.imageView = (imgInfo[i].instanced) ? imgInfo[i].pView[setInstance] : imgInfo[i].pView[0];
			vkInfo.sampler = imgInfo[i].sampler;

			vkImageInfos[i] = vkInfo;
		}
		for (size_t i = 0; i < bufInfo.size(); i++)
		{
			VkDescriptorBufferInfo vkInfo = {};
			vkInfo.buffer = (bufInfo[i].instanced) ? bufInfo[i].pBuffer[setInstance] : bufInfo[i].pBuffer[0];
			vkInfo.offset = bufInfo[i].offset;
			vkInfo.range = bufInfo[i].range;

			vkBufferInfos[i] = vkInfo;
		}

		//descriptor writes
		for (size_t i = 0; i < imgInfo.size(); i++)
		{
			uint32_t currentBinding = imgInfo[i].binding;
			descriptorWrites[currentBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[currentBinding].dstSet = allocatedDSs[setInstance];
			descriptorWrites[currentBinding].dstBinding = currentBinding;
			descriptorWrites[currentBinding].descriptorType = DSLBindings[currentBinding].descriptorType;
			descriptorWrites[currentBinding].descriptorCount = 1;
			descriptorWrites[currentBinding].pImageInfo = &vkImageInfos[i];
		}		
		for (size_t i = 0; i < bufInfo.size(); i++)
		{			
			uint32_t currentBinding = bufInfo[i].binding;
			descriptorWrites[currentBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[currentBinding].dstSet = allocatedDSs[setInstance];
			descriptorWrites[currentBinding].dstBinding = currentBinding;
			descriptorWrites[currentBinding].descriptorType = DSLBindings[currentBinding].descriptorType;
			descriptorWrites[currentBinding].descriptorCount = 1;
			descriptorWrites[currentBinding].pBufferInfo = &vkBufferInfos[i];
		}
		//update descriptors of this instance
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}


SbDescriptorSet::SbDescriptorSet(const VkDevice& device, SbSwapchain& swapchain,
	const VkDescriptorSetLayout& DSL,
	const std::vector<VkDescriptorSetLayoutBinding>& DSLBindings)
	: device(device), swapchain(swapchain), 
	DSL(DSL), DSLBindings(DSLBindings)
{

}

SbDescriptorSet& SbDescriptorSet::addImageBinding(uint32_t binding, VkSampler sampler, VkImageView* imageView)
{
	imgInfo.push_back(
		SbImageInfo{ 
			binding,
			sampler, 
			imageView, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			false 
		});
	return *this;
}

SbDescriptorSet& SbDescriptorSet::addInputAttachmentBinding(uint32_t binding, uint32_t attachmentID)
{
	imgInfo.push_back(
		SbImageInfo{
			binding,
			VK_NULL_HANDLE,
			swapchain.getAttachmentViews(attachmentID).data(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			true
		});
	return *this;
}


void SbDescriptorSet::allocate(const SbDescriptorPool& descriptorPool)
{
	bool setIsInstanced = false;
	for (auto& i : imgInfo)
	{		
		if (i.instanced) {
			setIsInstanced = true;
			goto proceed;
		}
	}
	for (auto& i : bufInfo)
	{
		if (i.instanced) {
			setIsInstanced = true;
			goto proceed;
		}
	}
proceed:

	std::vector<VkDescriptorSetLayout> layouts;
	if (setIsInstanced) 
	{
		layouts = std::vector<VkDescriptorSetLayout>(swapchain.getSize(), DSL);
		allocatedDSs = std::vector<VkDescriptorSet>(swapchain.getSize());
	}
	else 
	{
		layouts = std::vector<VkDescriptorSetLayout>(1, DSL);
		allocatedDSs = std::vector<VkDescriptorSet>(1);
	}
	
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool.pool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	auto result = vkAllocateDescriptorSets(device, &allocInfo, allocatedDSs.data());
	if (result != VK_SUCCESS) {
		switch (result)
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("VK_ERROR_OUT_OF_HOST_MEMORY");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("VK_ERROR_OUT_OF_DEVICE_MEMORY");
		case VK_ERROR_FRAGMENTED_POOL:
			throw std::runtime_error("VK_ERROR_FRAGMENTED_POOL");
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			throw std::runtime_error("VK_ERROR_OUT_OF_POOL_MEMORY");
		default:
			break;
		}
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

}
