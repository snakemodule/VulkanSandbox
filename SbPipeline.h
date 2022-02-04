#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include "VulkanInitializers.hpp"
#include "SbShaderLayout.h"


//encapsulates pipeline creation using named parameter idiom.

class SbPipeline
{
private:
	//restore these, easier to read than in constructor
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI;
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI;
	//VkPipelineColorBlendAttachmentState defaultBlendAttachmentState;
	//VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI;// = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
	VkPipelineViewportStateCreateInfo viewportStateCI;// = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleStateCI;// = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables;// = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	//VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	VkGraphicsPipelineCreateInfo pipelineCI;// = vks::initializers::pipelineCreateInfo();
	std::array<VkPipelineShaderStageCreateInfo,2> shaderStages;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;

	
public:
	VkPipeline handle;

	

	SbPipeline& shaderLayouts(SbShaderLayout& shaderLayout);
	//VkDescriptorSetLayout getSetLayout(int set);

	//obligatory
	SbPipeline& subpassIndex(const uint32_t & subpass);
	//SbPipeline& addShaderStage(const VkPipelineShaderStageCreateInfo & shaderStage);
	SbPipeline& addBlendAttachmentState(VkPipelineColorBlendAttachmentState blend, uint32_t index);
	SbPipeline& addBlendAttachmentStates(VkPipelineColorBlendAttachmentState blend, uint32_t startIndex, uint32_t endIndex);

	//optional parameters
	SbPipeline& specializeFrag(VkSpecializationInfo*);
	SbPipeline& specializeVert(VkSpecializationInfo*);
	SbPipeline & vertexBindingDescription(const std::vector<VkVertexInputBindingDescription>& v);
	SbPipeline & vertexAttributeDescription(const std::vector<VkVertexInputAttributeDescription>& v);
	SbPipeline& cullMode(const VkCullModeFlags & flags);
	SbPipeline& depthWriteEnable(const VkBool32 & enable);
	SbPipeline& colorBlending(uint32_t attachmentIndex);

	VkPipelineLayout getLayout();

	void createPipeline(const VkRenderPass & renderPass, const VkDevice & device);

	SbPipeline();
	~SbPipeline();
};

