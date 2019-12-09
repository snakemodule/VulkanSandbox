#include "SbDescriptorSets.h"

#include "VulkanInitializers.hpp"
#include <algorithm>
#include <tuple>


//#include <iterator>

// todo secure pointers
void SbDescriptorSets::updateDescriptors()
{
	

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	for (size_t i = 0; i < allocatedDSs.size(); i++)
	{
		for (auto& p : bindings) {
			if (imgInfo.find(p.first) != imgInfo.end())
			{
				auto & info = imgInfo.find(p.first)->second;
				VkImageView view = (info.mode == eBindingMode_Shared) ? info.pView[0] : info.pView[i];
				VkDescriptorImageInfo imageInfo = { info.sampler, view, info.layout };
				descriptorWrites.push_back(vks::initializers::writeDescriptorSet(allocatedDSs[i],
					p.second.descriptorType, p.second.binding, &imageInfo));
			}
			if (bufInfo.find(p.first) != bufInfo.end())
			{
				auto & info = bufInfo.find(p.first)->second;
				VkBuffer buffer = (info.mode == eBindingMode_Shared) ? info.pBuffer[0] : info.pBuffer[i];
				VkDescriptorBufferInfo bufferInfo = { buffer, info.offset, info.range };
				descriptorWrites.push_back(vks::initializers::writeDescriptorSet(allocatedDSs[i],
					p.second.descriptorType, p.second.binding, &bufferInfo));
			}
		}
	}
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

SbDescriptorSets::SbDescriptorSets(const VkDevice & device, const uint32_t & descriptorSetCount)
	: device(device), allocatedDSs(descriptorSetCount)
{

}

SbDescriptorSets::~SbDescriptorSets()
{
}

void SbDescriptorSets::addImageBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbImageInfo & imageInfo)
{
	bindings.insert(std::make_pair(newBinding.binding, newBinding));
	imgInfo.insert(std::make_pair(newBinding.binding, imageInfo));
}

void SbDescriptorSets::addBufferBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbBufferInfo & bufferInfo)
{
	bindings.insert(std::make_pair(newBinding.binding, newBinding));
	bufInfo.insert(std::make_pair(newBinding.binding, bufferInfo));
}

void SbDescriptorSets::allocateDescriptorSets(const SbDescriptorPool & descriptorPool)
{
	std::vector<VkDescriptorSetLayout> layouts(allocatedDSs.size(), DSLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool.pool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device, &allocInfo, allocatedDSs.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	
}

std::vector<VkDescriptorPoolSize> SbDescriptorSets::getRequiredPoolSizesForBindings()
{
	std::vector<VkDescriptorPoolSize> resultVector;
	std::for_each(bindings.begin(), bindings.end(), [&](std::pair<const uint32_t, const VkDescriptorSetLayoutBinding> & KVpair) {
		auto & binding = KVpair.second;
		VkDescriptorPoolSize ps = { binding.descriptorType, binding.binding };
		resultVector.push_back(ps);
	});
	return resultVector;
}

std::vector<VkDescriptorSetLayoutBinding> SbDescriptorSets::bindingsAsVector()
{
	std::vector<VkDescriptorSetLayoutBinding> resultVector;
	for (auto item : bindings) {
		auto & binding = item.second;
		resultVector.push_back(binding);
	}
	return resultVector;
}


void SbDescriptorSets::createDSLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindingsVector = bindingsAsVector();
	VkDescriptorSetLayoutCreateInfo DS_Layout_CI = vks::initializers::descriptorSetLayoutCreateInfo(bindingsVector);
	if (vkCreateDescriptorSetLayout(device, &DS_Layout_CI, nullptr, &DSLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void SbDescriptorSets::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&DSLayout);
	if (vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
