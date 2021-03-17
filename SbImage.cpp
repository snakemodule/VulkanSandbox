#include "SbImage.h"

#include "SbVulkanBase.h"

SbImage::SbImage(SbVulkanBase& base, uint32_t width, uint32_t height, uint32_t mipLevels,
	VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties)
	: width(width), height(height), mipLevels(mipLevels)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	img = base.getDevice().createImage(imageInfo);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(base.getDevice(), img, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = base.findMemoryType(memRequirements.memoryTypeBits, properties);

	memory = base.getDevice().allocateMemory(allocInfo);

	base.getDevice().bindImageMemory(img, memory, 0);
}


void SbImage::Destroy(vk::Device device) {
	device.destroyImage(img);
	device.freeMemory(memory);
}