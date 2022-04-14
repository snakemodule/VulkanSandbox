#pragma once

class SbVulkanBase;
//#include "SbCommandPool.fwd.h"
//#include "SbVulkanBase.fwd.h"


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
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel = 0, uint32_t bufferOffset = 0);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//todo move to a texture class?
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

	

};

