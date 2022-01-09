#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"

class SbVulkanBase;


/// <summary>
/// vk::image bound to device memory
/// </summary>
class SbImage
{
public:	
	vk::Image img;
	vk::DeviceMemory memory;

	int width, height;
	vk::DeviceSize imageSize;
	uint32_t mipLevels;

	VkImageCreateInfo imageInfo = {};

	SbImage(SbVulkanBase& base, uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties);


	void Destroy(vk::Device device);
	
	

};

