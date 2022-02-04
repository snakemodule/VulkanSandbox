#pragma once

#include "vulkan/vulkan.h"

#include <fstream>
#include <assert.h>
#include <iostream>

namespace vks
{
	namespace helper
	{
		VkShaderModule loadShader(const char* fileName, VkDevice device);

		VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice device);

		//todo flytta till SbVulkanBase?
		VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);


		

	}
}