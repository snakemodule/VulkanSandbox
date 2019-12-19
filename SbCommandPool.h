#pragma once

#include "SbVulkanBase.h"

class SbCommandPool
{
public:

	VkCommandPool handle;

	SbPhysicalDevice & physicalDevice;
	SbLogicalDevice & logicalDevice;

	SbCommandPool(SbVulkanBase & base);
	~SbCommandPool();

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	//todo temporary place for these functions
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//todo move to a texture class?
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	

};

