#include "SbPipeline.h"

#include <array>
#include "VulkanHelperFunctions.hpp"

#include "VulkanInitializers.hpp"

SbPipeline & SbPipeline::vertexBindingDescription(const std::vector<VkVertexInputBindingDescription> & v)
{
	vertexBindingDescriptions = v;
	return *this;
}

SbPipeline & SbPipeline::vertexAttributeDescription(const std::vector<VkVertexInputAttributeDescription> & v)
{
	vertexAttributeDescriptions = v;
	return *this;
}

SbPipeline & SbPipeline::layout(const VkPipelineLayout & pipelineLayout)
{
	pipelineCI.layout = pipelineLayout;
	return *this;
}

SbPipeline & SbPipeline::subpassIndex(const uint32_t & subpass)
{
	pipelineCI.subpass = subpass;
	return *this;
}

SbPipeline & SbPipeline::cullMode(const VkCullModeFlags  & flags)
{
	rasterizationStateCI.cullMode = flags;
	return *this;
}

SbPipeline & SbPipeline::depthWriteEnable(const VkBool32 & enable)
{
	depthStencilStateCI.depthWriteEnable = enable;
	return *this;
}



SbPipeline & SbPipeline::addShaderStage(const VkPipelineShaderStageCreateInfo & shaderStage)
{
	shaderStages.push_back(shaderStage);
	return *this;
}

void SbPipeline::createPipeline(const VkRenderPass & renderPass, const VkDevice & device)
{

	pipelineCI.renderPass = renderPass;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pDynamicState = &vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables); //&dynamicStateCI;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();


	auto & bind = vertexBindingDescriptions;
	auto & attr = vertexAttributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = vks::initializers::pipelineVertexInputStateCreateInfo();
	if (bind.size())
	{
		vertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(bind.size());
		vertexInputStateCI.pVertexBindingDescriptions = bind.data();
	}
	if (attr.size())
	{
		vertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
		vertexInputStateCI.pVertexAttributeDescriptions = attr.data();
	}
	pipelineCI.pVertexInputState = &vertexInputStateCI;



	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &handle);


}

SbPipeline::SbPipeline()
	: inputAssemblyStateCI {vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE) },
	rasterizationStateCI {vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0)},
	blendAttachmentState {vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)},
	depthStencilStateCI	{vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)},
	viewportStateCI	{vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0)},
	multisampleStateCI{vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0)},
	dynamicStateEnables { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR },
	pipelineCI { vks::initializers::pipelineCreateInfo() }
{
}


SbPipeline::~SbPipeline()
{
}
