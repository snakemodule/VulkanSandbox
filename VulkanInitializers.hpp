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
#include "vulkan/vulkan.hpp"

namespace vks
{
	namespace initializers
	{
		vk::PipelineShaderStageCreateInfo pipeline_shader_stage_create_info(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule);


		vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
			vk::CommandPool commandPool,
			vk::CommandBufferLevel level,
			uint32_t bufferCount);

		vk::CommandPoolCreateInfo commandPoolCreateInfo();

		vk::CommandBufferBeginInfo commandBufferBeginInfo();

		vk::CommandBufferInheritanceInfo commandBufferInheritanceInfo();

		vk::RenderPassBeginInfo renderPassBeginInfo();

		vk::RenderPassCreateInfo renderPassCreateInfo();

		/** @brief Initialize an image memory barrier with no image transfer ownership */
		vk::ImageMemoryBarrier imageMemoryBarrier();

		/** @brief Initialize a buffer memory barrier with no image transfer ownership */
		vk::BufferMemoryBarrier bufferMemoryBarrier();

		vk::MemoryBarrier memoryBarrier();

		vk::ImageCreateInfo imageCreateInfo();

		vk::SamplerCreateInfo samplerCreateInfo();

		vk::ImageViewCreateInfo imageViewCreateInfo();

		vk::FramebufferCreateInfo framebufferCreateInfo();

		vk::SemaphoreCreateInfo semaphoreCreateInfo();

		vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlags flags);

		vk::EventCreateInfo eventCreateInfo();

		vk::SubmitInfo submitInfo();

		vk::Viewport viewport(
			float width,
			float height,
			float minDepth,
			float maxDepth);

		vk::Rect2D rect2D(
			int32_t width,
			int32_t height,
			int32_t offsetX,
			int32_t offsetY);

		vk::Rect2D rect2D(
			vk::Extent2D extent,
			int32_t offsetX,
			int32_t offsetY);

		vk::BufferCreateInfo bufferCreateInfo();

		vk::BufferCreateInfo bufferCreateInfo(
			vk::BufferUsageFlags usage,
			vk::DeviceSize size);

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
			uint32_t poolSizeCount,
			vk::DescriptorPoolSize* pPoolSizes,
			uint32_t maxSets);

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
			const std::vector<vk::DescriptorPoolSize>& poolSizes,
			uint32_t maxSets);

		vk::DescriptorPoolSize descriptorPoolSize(
			vk::DescriptorType type,
			uint32_t descriptorCount);

		vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
			vk::DescriptorType type,
			vk::ShaderStageFlags stageFlags,
			uint32_t binding,
			uint32_t descriptorCount = 1);

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const vk::DescriptorSetLayoutBinding* pBindings, uint32_t bindingCount);

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const std::vector<VkDescriptorSetLayoutBinding>& bindings); //todo deprecate this

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo();

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			const vk::DescriptorSetLayout* pSetLayouts,
			uint32_t setLayoutCount = 1);


		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(
			vk::DescriptorPool descriptorPool,
			const vk::DescriptorSetLayout* pSetLayouts,
			uint32_t descriptorSetCount);

		vk::DescriptorImageInfo descriptorImageInfo(vk::Sampler sampler, vk::ImageView imageView, vk::ImageLayout imageLayout);

		vk::WriteDescriptorSet writeDescriptorSet(
			vk::DescriptorSet dstSet,
			vk::DescriptorType type,
			uint32_t binding,
			vk::DescriptorBufferInfo* bufferInfo,
			uint32_t descriptorCount = 1);

		vk::WriteDescriptorSet writeDescriptorSet(
			vk::DescriptorSet dstSet,
			vk::DescriptorType type,
			uint32_t binding,
			vk::DescriptorImageInfo* imageInfo,
			uint32_t descriptorCount = 1);

		vk::VertexInputBindingDescription vertexInputBindingDescription(
			uint32_t binding,
			uint32_t stride,
			vk::VertexInputRate inputRate);

		vk::VertexInputAttributeDescription vertexInputAttributeDescription(
			uint32_t binding,
			uint32_t location,
			vk::Format format,
			uint32_t offset);

		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();

		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			vk::PrimitiveTopology topology,
			vk::PipelineInputAssemblyStateCreateFlags flags = vk::PipelineInputAssemblyStateCreateFlags(),
			vk::Bool32 primitiveRestartEnable = false);

		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			vk::PolygonMode polygonMode,
			vk::CullModeFlags cullMode,
			vk::FrontFace frontFace,
			vk::PipelineRasterizationStateCreateFlags flags = vk::PipelineRasterizationStateCreateFlags());

		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(vk::PolygonMode polygonMode);


		vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState();


		vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			vk::ColorComponentFlags colorWriteMask,
			vk::Bool32 blendEnable);

		vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			uint32_t attachmentCount,
			const vk::PipelineColorBlendAttachmentState* pAttachments);

		vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
			vk::Bool32 depthTestEnable,
			vk::Bool32 depthWriteEnable,
			vk::CompareOp depthCompareOp);

		vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
			uint32_t viewportCount,
			uint32_t scissorCount,
			vk::PipelineViewportStateCreateFlags flags = vk::PipelineViewportStateCreateFlags());

		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo();


		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			vk::SampleCountFlagBits rasterizationSamples,
			vk::PipelineMultisampleStateCreateFlags flags = vk::PipelineMultisampleStateCreateFlags());

		vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const vk::DynamicState* pDynamicStates,
			uint32_t dynamicStateCount,
			vk::PipelineDynamicStateCreateFlags flags = vk::PipelineDynamicStateCreateFlags());

		vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const std::vector<vk::DynamicState>& pDynamicStates,
			vk::PipelineDynamicStateCreateFlags flags = vk::PipelineDynamicStateCreateFlags());

		vk::PipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints);

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
			vk::PipelineLayout layout,
			vk::RenderPass renderPass,
			vk::PipelineCreateFlags flags = vk::PipelineCreateFlags());

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo();

		vk::ComputePipelineCreateInfo computePipelineCreateInfo(
			vk::PipelineLayout layout,
			vk::PipelineCreateFlags flags = vk::PipelineCreateFlags());

		vk::PushConstantRange pushConstantRange(
			vk::ShaderStageFlags stageFlags,
			uint32_t size,
			uint32_t offset);

		vk::BindSparseInfo bindSparseInfo();

		/** @brief Initialize a map entry for a shader specialization constant */
		vk::SpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size);

		/** @brief Initialize a specialization constant info structure to pass to a shader stage */
		vk::SpecializationInfo specializationInfo(uint32_t mapEntryCount, const vk::SpecializationMapEntry* mapEntries, size_t dataSize, const void* data);
	}
}