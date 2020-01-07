#include "SbPipelineLayout.h"

#include "VulkanInitializers.hpp"
#include <algorithm>
#include <tuple>


//#include <iterator>

// todo secure pointers
void SbPipelineLayout::updateDescriptors()
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
		std::vector<VkDescriptorBufferInfo> bufferInfos(bufInfo.size());
		std::vector<VkDescriptorImageInfo> imageInfos(imgInfo.size());

		int infoInsertCounter = 0;
		for (auto& pair : imgInfo) {
			SbImageInfo & info = pair.second;
			int bindingIndex = pair.first;
			auto binding = bindings.find(bindingIndex)->second;
			
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = info.layout;// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = (info.mode == SbPipelineLayout::eSharingMode_Shared) ? info.pView[0] : info.pView[i];//textureImageView;
			imageInfo.sampler = info.sampler;// textureSampler;
			imageInfos[infoInsertCounter] = imageInfo;

			descriptorWrites[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[bindingIndex].dstSet = allocatedDSs[i];
			descriptorWrites[bindingIndex].dstBinding = binding.binding;
			descriptorWrites[bindingIndex].descriptorType = binding.descriptorType;//VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[bindingIndex].descriptorCount = 1;
			descriptorWrites[bindingIndex].pImageInfo = &imageInfos[infoInsertCounter];
			++infoInsertCounter;
		}

		infoInsertCounter = 0;
		for (auto& pair : bufInfo) {
			SbBufferInfo & info = pair.second;
			int bindingIndex = pair.first;
			auto binding = bindings.find(bindingIndex)->second;

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = (info.mode == SbPipelineLayout::eSharingMode_Shared) ? info.pBuffer[0] : info.pBuffer[i];//uniformBuffers[i];
			bufferInfo.offset = info.offset;
			bufferInfo.range = info.range;
			bufferInfos[infoInsertCounter] = bufferInfo;

			descriptorWrites[binding.binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[binding.binding].dstSet = allocatedDSs[i];//descriptorSets.attachmentWrite[i];
			descriptorWrites[binding.binding].dstBinding = binding.binding;//0;
			descriptorWrites[binding.binding].descriptorType = binding.descriptorType;// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[binding.binding].descriptorCount = 1;
			descriptorWrites[binding.binding].pBufferInfo = &bufferInfos[infoInsertCounter];//&bufferInfo;
			++infoInsertCounter;
		}
		
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}


}

SbPipelineLayout::SbPipelineLayout(const VkDevice & device, const uint32_t & descriptorSetCount)
	: device(device), allocatedDSs(descriptorSetCount)
{

}

SbPipelineLayout::~SbPipelineLayout()
{
}

void SbPipelineLayout::addImageBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbImageInfo & imageInfo)
{
	bindings.insert(std::make_pair(newBinding.binding, newBinding));
	imgInfo.insert(std::make_pair(newBinding.binding, imageInfo));
}


void SbPipelineLayout::allocateDescriptorSets(const SbDescriptorPool & descriptorPool)
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

std::vector<VkDescriptorPoolSize> SbPipelineLayout::getRequiredPoolSizesForBindings()
{
	std::vector<VkDescriptorPoolSize> resultVector;
	std::for_each(bindings.begin(), bindings.end(), [&](std::pair<const uint32_t, VkDescriptorSetLayoutBinding> & KVpair) {
		auto & binding = KVpair.second;
		VkDescriptorPoolSize ps = { binding.descriptorType, binding.binding };
		resultVector.push_back(ps);
	});
	return resultVector;
}

std::vector<VkDescriptorSetLayoutBinding> SbPipelineLayout::bindingsAsVector()
{
	std::vector<VkDescriptorSetLayoutBinding> resultVector;
	for (auto pair : bindings) {
		auto & binding = pair.second;
		resultVector.push_back(binding);
	}
	return resultVector;
}


void SbPipelineLayout::createDSLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindingsVector = bindingsAsVector();
	VkDescriptorSetLayoutCreateInfo DS_Layout_CI = vks::initializers::descriptorSetLayoutCreateInfo(bindingsVector);
	if (vkCreateDescriptorSetLayout(device, &DS_Layout_CI, nullptr, &DSLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void SbPipelineLayout::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&DSLayout);
	if (vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
