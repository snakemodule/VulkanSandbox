#pragma once

#include "SbRenderpass.h"

//#include <array>

class MyRenderPass :
    public SbRenderpass
{
public:
	enum
	{
		kSubpass_GBUF,
		kSubpass_COMPOSE,
		kSubpass_TRANSPARENT,

		kSubpass_COUNT,
		kSubpass_MAX = kSubpass_COUNT - 1
	};

	enum
	{
		kAttachment_BACK, //is this ever needed?
		kAttachment_POSITION,
		kAttachment_NORMAL,
		kAttachment_ALBEDO,
		kAttachment_DEPTH,

		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};

	using AttachmentFormat = std::pair<VkFormat, VkImageUsageFlags>;

	std::array<AttachmentFormat, kAttachment_COUNT> fillFormatTable()
	{
		std::array<AttachmentFormat, kAttachment_COUNT> table = { };
		table[kAttachment_POSITION] = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		table[kAttachment_NORMAL] = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		table[kAttachment_ALBEDO] = { VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };

		return table;
	}

	std::array<AttachmentFormat, kAttachment_COUNT> attachmentFormatTable = fillFormatTable();


	MyRenderPass(SbVulkanBase& base, SbSwapchain& swapchain)
		: SbRenderpass(base, kSubpass_COUNT, kAttachment_COUNT, swapchain.getSize())
	{
		attachmentFormatTable[kAttachment_DEPTH] = 
			{ base.findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT };

		setUpSwapchainAttachments(swapchain);
		setUpRenderpass(swapchain);
	};




	void setUpSwapchainAttachments(SbSwapchain& swapchain) {
		swapchain.prepareAttachmentSets(kAttachment_COUNT);
		createAttachmentFor(swapchain, kAttachment_POSITION);
		createAttachmentFor(swapchain, kAttachment_NORMAL);
		createAttachmentFor(swapchain, kAttachment_ALBEDO);
		createAttachmentFor(swapchain, kAttachment_DEPTH);
	}

private:
    void setUpRenderpass(SbSwapchain& swapchain) override;
	void createAttachmentFor(SbSwapchain& swapchain, int attachmentIndex);
};

