#pragma once

#include "vulkan/vulkan.hpp"

#include <map>

#include "SbUniformBuffer.h"
//#include "SbSwapchain.h"

#include "SbShaderLayout.h"

//this is an allocated descriptorset

#include "SbRenderpass.h"
#include "SbFramebuffer.h"

class SbDescriptorPool;

class SbDescriptorSet
{
public:
	struct SbImageInfo {
		const uint32_t binding;
		const VkSampler sampler;
		const std::vector<VkImageView> view; //const VkImageView* pView; 
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

	std::vector<VkDescriptorSetLayoutBinding> bindingInfo;
	VkDescriptorSetLayout setLayout;

	const VkDevice device;
		
	SbDescriptorSet(const VkDevice& device, SbSwapchain& swapchain, SbShaderLayout& shaderLayout, uint32_t set);

	void updateDescriptors();

	SbDescriptorSet& addImageBinding(uint32_t binding, VkSampler sampler, VkImageView imageView);
	SbDescriptorSet& addInputAttachmentBinding(uint32_t binding, uint32_t attachmentID, std::vector<SbFramebuffer> framebufferInstances);

	void allocate(const SbDescriptorPool& descriptorPool);

	//std::vector<VkDescriptorPoolSize> getRequiredPoolSizesForBindings();
	//void createDSLayout();
	//void createPipelineLayout();
	

	template <class T>
	void addBufferBinding(uint32_t binding, const SbUniformBuffer<T>* buffer) {
		//todo how to determine instanced mode?
		bool instanced = buffer->buffers.size() > 1;
		SbBufferInfo info = { binding, buffer->buffers.data(), 0, buffer->bufferSize, instanced };
		bufInfo.push_back(info);
	};

};

