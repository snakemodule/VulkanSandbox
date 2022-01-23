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
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI;// = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI;// = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	//VkPipelineColorBlendAttachmentState defaultBlendAttachmentState;// = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	//VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI;// = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
	VkPipelineViewportStateCreateInfo viewportStateCI;// = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleStateCI;// = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables;// = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	//VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	VkGraphicsPipelineCreateInfo pipelineCI;// = vks::initializers::pipelineCreateInfo();
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
public:
	VkPipeline handle;

	SbShaderLayout shaderLayout;

	SbPipeline& shaderLayouts(vk::Device device, std::string vert, std::string frag);
	SbPipeline& shaderStageSpecialization(size_t stage, const VkSpecializationInfo* specializationInfo);
	//VkDescriptorSetLayout getSetLayout(int set);

	//obligatory
	SbPipeline& layout(const VkPipelineLayout & pipelineLayout);
	SbPipeline& subpassIndex(const uint32_t & subpass);
	SbPipeline& addShaderStage(const VkPipelineShaderStageCreateInfo & shaderStage);
	SbPipeline& addBlendAttachmentState(VkPipelineColorBlendAttachmentState blend, uint32_t index);
	SbPipeline& addBlendAttachmentStates(VkPipelineColorBlendAttachmentState blend, uint32_t startIndex, uint32_t endIndex);

	//optional parameters
	SbPipeline & vertexBindingDescription(const std::vector<VkVertexInputBindingDescription>& v);
	SbPipeline & vertexAttributeDescription(const std::vector<VkVertexInputAttributeDescription>& v);
	SbPipeline& cullMode(const VkCullModeFlags & flags);
	SbPipeline& depthWriteEnable(const VkBool32 & enable);
	SbPipeline& colorBlending(uint32_t attachmentIndex);


	void createPipeline(const VkRenderPass & renderPass, const VkDevice & device, uint32_t subpass);

	const VkDescriptorSetLayout& getDSL(uint32_t set)
	{
		return shaderLayout.DSL[set];
	}

	const std::vector<VkDescriptorSetLayoutBinding>& getDSLBindings(uint32_t set)
	{
		return shaderLayout.bindings[set];
	}

	SbPipeline();
	~SbPipeline();
};

