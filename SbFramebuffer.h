#pragma once

#include "vulkan/vulkan.hpp"
#include "VulkanInitializers.hpp"
#include "SbVulkanBase.h"

#include "RenderpassHelper.h"

#include <cassert>

class SbFramebuffer
{
public:
	// Framebuffer for offscreen rendering
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	VkExtent2D extent;
	VkFramebuffer frameBuffer;
	VkRenderPass renderpass;

	std::vector <VkImage> images;
	std::vector <VkDeviceMemory> imageMemory;
	std::vector <VkImageView> views;
	
	void addAttachmentImage(uint32_t attachmentIndex, VkImageView imageView);

	void createAttachmentImage(SbVulkanBase* base, RenderpassHelper& rp, uint32_t attachmentIndex);

	void createFramebuffer(VkDevice device);

	SbFramebuffer(VkExtent2D extent, RenderpassHelper rp);



};

