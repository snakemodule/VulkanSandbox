#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class SbRenderpassAttachment
{
public:
	SbRenderpassAttachment(size_t size);
	~SbRenderpassAttachment();


	VkAttachmentDescription description;

	std::vector<VkImage> image;
	std::vector<VkDeviceMemory> mem;
	std::vector<VkImageView> view;
	VkFormat format;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

};

