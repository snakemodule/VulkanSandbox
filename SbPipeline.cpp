#include "SbPipeline.h"

#include <array>
#include "VulkanHelperFunctions.hpp"

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
	pipelineCI.pColorBlendState = &colorBlendStateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;
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
{
}


SbPipeline::~SbPipeline()
{
}
