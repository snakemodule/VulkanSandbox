#include "SbPipeline.h"

#include <array>
#include "VulkanHelperFunctions.h"

#include "VulkanInitializers.hpp"

//#include "spirv_reflect.hpp"
//#include <utility>

SbPipeline & SbPipeline::vertexBindingDescription(const std::vector<vk::VertexInputBindingDescription>& v)
{
	vertexBindingDescriptions = v;
	return *this;
}

SbPipeline & SbPipeline::vertexAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& v)
{
	vertexAttributeDescriptions = v;
	return *this;
}

SbPipeline& SbPipeline::shaderLayouts(vk::Device device, std::string vert, std::string frag)
{
	pipelineCI.layout = shaderLayout.reflect(device, vert, frag, shaderStages);
	return *this;
}

SbPipeline& SbPipeline::shaderStageSpecialization(size_t stage, const vk::SpecializationInfo * specializationInfo)
{
	shaderStages[stage].pSpecializationInfo = specializationInfo;
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

SbPipeline & SbPipeline::cullMode(const vk::CullModeFlags & flags)
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

	vk::PipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA, true); //todo bits better than 0xf?
	blendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrc1Alpha;
	blendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrc1Alpha;
	blendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
	blendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	blendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	blendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;
	blendAttachmentStates[attachmentIndex] = blendAttachmentState;
	return *this;
}

SbPipeline & SbPipeline::addShaderStage(const VkPipelineShaderStageCreateInfo & shaderStageCI)
{
	//reflect
	//uniform buffers
	//sampled images (texture samplers)
	//subpass inputs (image attachment)s

	shaderStages.push_back(shaderStageCI);
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

void SbPipeline::createPipeline(const vk::RenderPass & renderPass, const vk::Device & device, uint32_t subpass)
{
	pipelineCI.renderPass = renderPass;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &vks::initializers::pipelineColorBlendStateCreateInfo(
		static_cast<uint32_t>(blendAttachmentStates.size()), blendAttachmentStates.data());
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pDynamicState = &vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables); //&dynamicStateCI;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.subpass = subpass;
	
	auto & bind = vertexBindingDescriptions;
	auto & attr = vertexAttributeDescriptions;
	vertexInputStateCI = vks::initializers::pipelineVertexInputStateCreateInfo();
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




	handle = device.createGraphicsPipeline(vk::PipelineCache(), pipelineCI);
	//vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &handle);
}

SbPipeline::SbPipeline()
	: inputAssemblyStateCI{ vks::initializers::pipelineInputAssemblyStateCreateInfo(
		vk::PrimitiveTopology::eTriangleList) },
	rasterizationStateCI{ vks::initializers::pipelineRasterizationStateCreateInfo(
		vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise) },
	blendAttachmentStates {vks::initializers::pipelineColorBlendAttachmentState(
		vk::ColorComponentFlags(), false)},
	depthStencilStateCI	{vks::initializers::pipelineDepthStencilStateCreateInfo(
		true, true, vk::CompareOp::eLessOrEqual)},
	viewportStateCI	{vks::initializers::pipelineViewportStateCreateInfo(1, 1)},
	multisampleStateCI{vks::initializers::pipelineMultisampleStateCreateInfo(
		vk::SampleCountFlagBits::e1)},
	dynamicStateEnables { vk::DynamicState::eViewport, vk::DynamicState::eScissor },
	pipelineCI { vks::initializers::pipelineCreateInfo() }
{
}


SbPipeline::~SbPipeline()
{
}
