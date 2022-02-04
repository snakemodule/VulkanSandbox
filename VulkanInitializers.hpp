/*
* Initializers for Vulkan structures and objects used by the examples
* Saves lot of VK_STRUCTURE_TYPE assignments
* Some initializers are parameterized for convenience
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <vector>
#include "vulkan/vulkan.h"

namespace vks
{
	namespace initializers
	{

		VkMemoryAllocateInfo memoryAllocateInfo();

		VkMappedMemoryRange mappedMemoryRange();

		VkCommandBufferAllocateInfo commandBufferAllocateInfo(
			VkCommandPool commandPool, 
			VkCommandBufferLevel level, 
			uint32_t bufferCount);

		VkCommandPoolCreateInfo commandPoolCreateInfo();

		VkCommandBufferBeginInfo commandBufferBeginInfo();

		VkCommandBufferInheritanceInfo commandBufferInheritanceInfo();

		VkRenderPassBeginInfo renderPassBeginInfo();

		VkRenderPassCreateInfo renderPassCreateInfo();

		/** @brief Initialize an image memory barrier with no image transfer ownership */
		VkImageMemoryBarrier imageMemoryBarrier();

		/** @brief Initialize a buffer memory barrier with no image transfer ownership */
		VkBufferMemoryBarrier bufferMemoryBarrier();

		VkMemoryBarrier memoryBarrier();

		VkImageCreateInfo imageCreateInfo();

		VkSamplerCreateInfo samplerCreateInfo();

		VkImageViewCreateInfo imageViewCreateInfo();

		VkFramebufferCreateInfo framebufferCreateInfo();

		VkSemaphoreCreateInfo semaphoreCreateInfo();

		VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

		VkEventCreateInfo eventCreateInfo();

		VkSubmitInfo submitInfo();

		VkViewport viewport(
			float width,
			float height,
			float minDepth,
			float maxDepth);

		VkRect2D rect2D(
			int32_t width,
			int32_t height,
			int32_t offsetX,
			int32_t offsetY);

		VkRect2D rect2D(
			VkExtent2D extent,
			int32_t offsetX,
			int32_t offsetY);

		VkBufferCreateInfo bufferCreateInfo();

		inline VkBufferCreateInfo bufferCreateInfo(
			VkBufferUsageFlags usage,
			VkDeviceSize size)
		{
			VkBufferCreateInfo bufCreateInfo{};
			bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufCreateInfo.usage = usage;
			bufCreateInfo.size = size;
			return bufCreateInfo;
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
			uint32_t poolSizeCount,
			VkDescriptorPoolSize* pPoolSizes,
			uint32_t maxSets);

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
			const std::vector<VkDescriptorPoolSize>& poolSizes,
			uint32_t maxSets);

		VkDescriptorPoolSize descriptorPoolSize(
			VkDescriptorType type,
			uint32_t descriptorCount);

		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
			VkDescriptorType type,
			VkShaderStageFlags stageFlags,
			uint32_t binding,
			uint32_t descriptorCount);

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const VkDescriptorSetLayoutBinding* pBindings,
			uint32_t bindingCount);

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const std::vector<VkDescriptorSetLayoutBinding>& bindings);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			const VkDescriptorSetLayout* pSetLayouts,
			uint32_t setLayoutCount);
				
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
			VkDescriptorPool descriptorPool,
			const VkDescriptorSetLayout* pSetLayouts,
			uint32_t descriptorSetCount);

		VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);

		VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorBufferInfo* bufferInfo,
			uint32_t descriptorCount = 1);

		VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorImageInfo *imageInfo,
			uint32_t descriptorCount = 1);

		VkVertexInputBindingDescription vertexInputBindingDescription(
			uint32_t binding,
			uint32_t stride,
			VkVertexInputRate inputRate);

		VkVertexInputAttributeDescription vertexInputAttributeDescription(
			uint32_t binding,
			uint32_t location,
			VkFormat format,
			uint32_t offset);

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			VkPrimitiveTopology topology,
			VkPipelineInputAssemblyStateCreateFlags flags,
			VkBool32 primitiveRestartEnable);

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			VkPolygonMode polygonMode,
			VkCullModeFlags cullMode,
			VkFrontFace frontFace,
			VkPipelineRasterizationStateCreateFlags flags = 0);

		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			VkColorComponentFlags colorWriteMask,
			VkBool32 blendEnable);

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			uint32_t attachmentCount,
			const VkPipelineColorBlendAttachmentState* pAttachments);

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
			VkBool32 depthTestEnable,
			VkBool32 depthWriteEnable,
			VkCompareOp depthCompareOp);

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
			uint32_t viewportCount,
			uint32_t scissorCount,
			VkPipelineViewportStateCreateFlags flags = 0);

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			VkSampleCountFlagBits rasterizationSamples,
			VkPipelineMultisampleStateCreateFlags flags = 0);

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const VkDynamicState * pDynamicStates,
			uint32_t dynamicStateCount,
			VkPipelineDynamicStateCreateFlags flags = 0);

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const std::vector<VkDynamicState>& pDynamicStates,
			VkPipelineDynamicStateCreateFlags flags = 0);

		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo(
			VkPipelineLayout layout,
			VkRenderPass renderPass,
			VkPipelineCreateFlags flags = 0);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo();

		VkComputePipelineCreateInfo computePipelineCreateInfo(
			VkPipelineLayout layout, 
			VkPipelineCreateFlags flags = 0);

		VkPushConstantRange pushConstantRange(
			VkShaderStageFlags stageFlags,
			uint32_t size,
			uint32_t offset);

		VkBindSparseInfo bindSparseInfo();

		/** @brief Initialize a map entry for a shader specialization constant */
		VkSpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size);

		/** @brief Initialize a specialization constant info structure to pass to a shader stage */
		VkSpecializationInfo specializationInfo(uint32_t mapEntryCount, const VkSpecializationMapEntry* mapEntries, size_t dataSize, const void* data);
	}
}