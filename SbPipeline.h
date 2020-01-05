#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "VulkanInitializers.hpp"

//encapsulates pipeline creation using named parameter idiom.
class SbPipeline
{
private:
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI;// = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI;// = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState;// = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
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
public:
	VkPipeline handle;


	//obligatory
	SbPipeline& layout(const VkPipelineLayout & pipelineLayout);
	SbPipeline& subpassIndex(const uint32_t & subpass);
	SbPipeline& addShaderStage(const VkPipelineShaderStageCreateInfo & shaderStage);


	//optional parameters
	SbPipeline & vertexBindingDescription(const std::vector<VkVertexInputBindingDescription>& v);
	SbPipeline & vertexAttributeDescription(const std::vector<VkVertexInputAttributeDescription>& v);
	SbPipeline& cullMode(const VkCullModeFlags & flags);
	SbPipeline& depthWriteEnable(const VkBool32 & enable);

	void createPipeline(const VkRenderPass & renderPass, const VkDevice & device);

	SbPipeline();
	~SbPipeline();
};

