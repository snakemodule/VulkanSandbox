
#include "vulkan/vulkan.h"

#include <fstream>
#include <assert.h>
#include <iostream>

#include "gli.hpp"
#include "VulkanInitializers.hpp"

namespace vks
{
	namespace helper
	{
		VkShaderModule loadShader(const char* fileName, VkDevice device);

		VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice device);

		//todo flytta till SbVulkanBase?
		VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange);
		
		void copyBufferToImage(VkCommandBuffer commandBuffer, gli::texture2d tex2D, uint32_t mipLevels, VkBuffer srcBuffer, VkImage dstImage);

		uint32_t findMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void createImageMemory(VkPhysicalDevice physDevice, VkDevice device, VkImage image, VkDeviceMemory* memory);
	}
}