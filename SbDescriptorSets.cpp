#include "SbDescriptorSets.h"

#include "VulkanInitializers.hpp"
#include <algorithm>
#include <tuple>


//#include <iterator>

// todo secure pointers
void SbDescriptorSets::updateDescriptors()
{
	
	/*
	for (size_t i = 0; i < allocatedDSs.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorBufferInfo dynamicBufferInfo = {};
		dynamicBufferInfo.buffer = shadingUniformBuffers[i];
		dynamicBufferInfo.offset = 0;
		dynamicBufferInfo.range = dynamicAlignment;

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;


		//todo get descriptorwrites for this layouy from SbLayout?
		std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = allocatedDSs[i];// descriptorSets.attachmentWrite[i];
		descriptorWrites[0].dstBinding = 0;
		//descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;


		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = DS.allocatedDSs[i];
		descriptorWrites[1].dstBinding = 1;
		//descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = DS.allocatedDSs[i];
		descriptorWrites[2].dstBinding = 2;
		//descriptorWrites[2].dstArrayElement = 0;
		//descriptorType must match the type of dstBinding
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &dynamicBufferInfo;
		descriptorWrites[0] = vks::initializers::writeDescriptorSet(attachmentWriteSubpass.allocatedDSs[i],
			bindings[0].descriptorType, bindings[0].binding, &bufferInfo);

		descriptorWrites[1] = vks::initializers::writeDescriptorSet(attachmentWriteSubpass.allocatedDSs[i],
			bindings[1].descriptorType, bindings[1].binding, &imageInfo);

		descriptorWrites[2] = vks::initializers::writeDescriptorSet(attachmentWriteSubpass.allocatedDSs[i],
			bindings[2].descriptorType, bindings[2].binding, &dynamicBufferInfo);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	*/
	

	for (size_t i = 0; i < allocatedDSs.size(); i++)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites(bindings.size());

		std::vector<VkDescriptorBufferInfo> bufferInfo;
		std::vector<VkDescriptorImageInfo> imageInfo;
		VkImageView view;
		VkBuffer buffer;
		for (auto& p : bindings) {
			auto bindingIndex = p.first;
			bufferInfo.clear();
			imageInfo.clear();
			if (imgInfo.find(bindingIndex) != imgInfo.end())
			{
				auto & info = imgInfo.find(bindingIndex)->second;
				view = (info.mode == eBindingMode_Shared) ? info.pView[0] : info.pView[i];
				imageInfo.push_back({ info.sampler, view, info.layout });
				descriptorWrites[p.second.binding] = vks::initializers::writeDescriptorSet(allocatedDSs[i], p.second.descriptorType, bindingIndex, &imageInfo.back());
			}
			if (bufInfo.find(bindingIndex) != bufInfo.end())
			{

				auto & info = bufInfo.find(bindingIndex)->second;
				buffer = (info.mode == eBindingMode_Shared) ? info.pBuffer[0] : info.pBuffer[i];
				bufferInfo.push_back({ buffer, info.offset, info.range });
				descriptorWrites[p.second.binding] = 
				vks::initializers::writeDescriptorSet(allocatedDSs[i], p.second.descriptorType, bindingIndex, &bufferInfo.back());
				/*
				{
					descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[1].dstSet = allocatedDSs[i];
					descriptorWrites[1].dstBinding = bindingIndex;
					//descriptorWrites[1].dstArrayElement = 0;
					descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[1].descriptorCount = 1;
					descriptorWrites[1].pImageInfo = //&imageInfo;
				};
				*/
			}
		}
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}


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
	std::for_each(bindings.begin(), bindings.end(), [&](std::pair<const uint32_t, VkDescriptorSetLayoutBinding> & KVpair) {
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
