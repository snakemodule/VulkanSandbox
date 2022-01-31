#include "SbPipeline.h"

#include <array>
#include "VulkanHelperFunctions.hpp"

#include "VulkanInitializers.hpp"

//#include "spirv_reflect.hpp"
//#include <utility>

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

SbPipeline& SbPipeline::shaderLayouts(vk::Device device, std::string vert, std::string frag)
{	
	pipelineCI.layout = shaderLayout.reflect(device, vert, frag, shaderStages);
	return *this;
}

//VkDescriptorSetLayout SbPipeline::getSetLayout(int set)
//{
//	return shaderLayout.setLayouts[set];
//}

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

SbPipeline & SbPipeline::colorBlending(uint32_t attachmentIndex)
{
	if (attachmentIndex > blendAttachmentStates.size() - 1)
	{
		blendAttachmentStates.resize(attachmentIndex + 1);
	}	

	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_TRUE); //todo bits better than 0xf?
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentStates[attachmentIndex] = blendAttachmentState;
	return *this;
}

SbPipeline & SbPipeline::addBlendAttachmentState(VkPipelineColorBlendAttachmentState blend, uint32_t index)
{
	if (index > blendAttachmentStates.size()-1 )
	{
		blendAttachmentStates.resize(index + 1);
	}
	blendAttachmentStates[index] = blend;
	return *this;
}

SbPipeline & SbPipeline::addBlendAttachmentStates(VkPipelineColorBlendAttachmentState blend, uint32_t startIndex, uint32_t endIndex)
{
	if (endIndex > blendAttachmentStates.size() - 1)
	{
		blendAttachmentStates.resize(endIndex + 1);
	}
	for (size_t i = startIndex; i <= endIndex; i++)
	{
		blendAttachmentStates[i] = blend;
	}
	return *this;
}

void SbPipeline::createPipeline(const VkRenderPass & renderPass, const VkDevice & device)
{

	auto& bind = vertexBindingDescriptions;
	auto& attr = vertexAttributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = vks::initializers::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &vertexInputStateCI;
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

	VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkPipelineColorBlendStateCreateInfo blendstateCI = vks::initializers::pipelineColorBlendStateCreateInfo(
		static_cast<uint32_t>(blendAttachmentStates.size()), blendAttachmentStates.data());

	pipelineCI.renderPass = renderPass;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &blendstateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();


	



	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &handle);


}

SbPipeline::SbPipeline()
	: inputAssemblyStateCI {vks::initializers::pipelineInputAssemblyStateCreateInfo(
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE) },
	rasterizationStateCI {vks::initializers::pipelineRasterizationStateCreateInfo(
		VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0)},
	blendAttachmentStates {vks::initializers::pipelineColorBlendAttachmentState(
		0xf, VK_FALSE)},
	depthStencilStateCI	{vks::initializers::pipelineDepthStencilStateCreateInfo(
		VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL)},
	viewportStateCI	{vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0)},
	multisampleStateCI{vks::initializers::pipelineMultisampleStateCreateInfo(
		VK_SAMPLE_COUNT_1_BIT, 0)},
	dynamicStateEnables { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR },
	pipelineCI { vks::initializers::pipelineCreateInfo() }
{
}


SbPipeline::~SbPipeline()
{
}
