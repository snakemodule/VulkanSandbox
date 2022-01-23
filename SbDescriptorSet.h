#pragma once

#include "vulkan/vulkan.hpp"

#include <map>

#include "SbUniformBuffer.h"
//#include "SbSwapchain.h"

#include "SbShaderLayout.h"

//this is an allocated descriptorset

#include "SbRenderpass.h"

class SbDescriptorPool;

class SbDescriptorSet
{
public:
	struct SbImageInfo {
		const uint32_t binding;
		const VkSampler sampler;
		const VkImageView* pView; //VkImageView* pView;
		const VkImageLayout layout;
		const bool instanced;
	};

	struct SbBufferInfo {
		const uint32_t binding;
		const VkBuffer* pBuffer;
		const VkDeviceSize offset;
		const VkDeviceSize range;
		const bool instanced;
	};

	std::vector<SbImageInfo> imgInfo;
	std::vector<SbBufferInfo> bufInfo;
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	std::vector<VkDescriptorSet> allocatedDSs;

	SbSwapchain& swapchain;
	//SbShaderLayout::SbSetLayout& shaderLayout;
	const VkDescriptorSetLayout& DSL;
	const std::vector<VkDescriptorSetLayoutBinding>& DSLBindings;

	const VkDevice device;
		
	SbDescriptorSet(const VkDevice& device, SbSwapchain& swapchain,
		const VkDescriptorSetLayout& DSL,
		const std::vector<VkDescriptorSetLayoutBinding>& DSLBindings);

	void updateDescriptors();

	SbDescriptorSet& addImageBinding(uint32_t binding, VkSampler sampler, VkImageView* imageView);
	SbDescriptorSet& addInputAttachmentBinding(uint32_t binding, uint32_t attachmentID);
	

	void allocate(const SbDescriptorPool& descriptorPool);

	//std::vector<VkDescriptorPoolSize> getRequiredPoolSizesForBindings();
	//void createDSLayout();
	//void createPipelineLayout();
	

	template <class T>
	void addBufferBinding(uint32_t binding, const SbUniformBuffer<T>& buffer) {
		//todo how to determine instanced mode?
		bool instanced = buffer.buffers.size() > 1;
		SbBufferInfo info = { binding, buffer.buffers.data(), 0, sizeof(T), instanced };
		bufInfo.push_back(info);
	};

};

