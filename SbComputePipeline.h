#pragma once

#include <vulkan/vulkan.hpp>
#include "VulkanInitializers.hpp"

#include "SbShaderLayout.h"

class SbComputePipeline
{
public:

	VkPipeline handle;

	VkComputePipelineCreateInfo pipelineCI;

	SbComputePipeline()
	{
	}

	void createPipeline(const VkDevice& device, SbShaderLayout& shaderLayout)
	{
		pipelineCI = vks::initializers::computePipelineCreateInfo(shaderLayout.results.pipelineLayout, 0);
		assert(shaderLayout.results.shaderInfo.size() == 1);
		pipelineCI.stage = shaderLayout.results.shaderInfo[0];

		vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &handle);
		//vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &handle);

		//shaderStages[0].pSpecializationInfo = nullptr;
		//shaderStages[1].pSpecializationInfo = nullptr;
	}

	VkPipelineLayout getLayout()
	{
		return pipelineCI.layout;
	}
};

