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

		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

		void transitionImageLayout(VkCommandBuffer cmd, VkImage image, 
			VkImageAspectFlagBits aspect, VkImageLayout oldLayout, VkImageLayout newLayout);
		

	}
}