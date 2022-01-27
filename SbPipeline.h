#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include "VulkanInitializers.hpp"

//encapsulates pipeline creation using named parameter idiom.

#include "SbShaderLayout.h"

class SbPipeline
{
private:
public: //TODO temporary public
	//restore these, easier to read than in constructor
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI;// = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	vk::PipelineRasterizationStateCreateInfo rasterizationStateCI;// = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	//VkPipelineColorBlendAttachmentState defaultBlendAttachmentState;// = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	//VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI;// = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
	vk::PipelineViewportStateCreateInfo viewportStateCI;// = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	vk::PipelineMultisampleStateCreateInfo multisampleStateCI;// = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<vk::DynamicState> dynamicStateEnables;// = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	//VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	vk::GraphicsPipelineCreateInfo pipelineCI;// = vks::initializers::pipelineCreateInfo();
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
	std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<vk::PipelineColorBlendAttachmentState> blendAttachmentStates;

	vk::PipelineVertexInputStateCreateInfo vertexInputStateCI;
public:
	VkPipeline handle;

	SbShaderLayout shaderLayout;

	SbPipeline& shaderLayouts(vk::Device device, std::string vert, std::string frag);
	SbPipeline& shaderStageSpecialization(size_t stage, const vk::SpecializationInfo * specializationInfo);
	//VkDescriptorSetLayout getSetLayout(int set);

	//obligatory
	SbPipeline& layout(const VkPipelineLayout & pipelineLayout);
	SbPipeline& subpassIndex(const uint32_t & subpass);
	SbPipeline& addShaderStage(const VkPipelineShaderStageCreateInfo & shaderStage);
	SbPipeline& addBlendAttachmentState(VkPipelineColorBlendAttachmentState blend, uint32_t index);
	SbPipeline& addBlendAttachmentStates(VkPipelineColorBlendAttachmentState blend, uint32_t startIndex, uint32_t endIndex);

	//optional parameters
	SbPipeline & vertexBindingDescription(const std::vector<vk::VertexInputBindingDescription>& v);
	SbPipeline & vertexAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& v);
	SbPipeline& cullMode(const vk::CullModeFlags & flags);
	SbPipeline& depthWriteEnable(const VkBool32 & enable);
	SbPipeline& colorBlending(uint32_t attachmentIndex);


	void createPipeline(const vk::RenderPass & renderPass, const vk::Device & device, uint32_t subpass);

	const vk::DescriptorSetLayout& getDSL(uint32_t set)
	{
		return shaderLayout.DSL[set];
	}

	const std::vector<vk::DescriptorSetLayoutBinding>& getDSLBindings(uint32_t set)
	{
		return shaderLayout.bindings[set];
	}

	SbPipeline();
	~SbPipeline();
};

