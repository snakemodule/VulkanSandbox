#pragma once
#include "SbRenderpass.h"
class ShadowRenderpass :
    public SbRenderpass
{

	enum
	{
		kSubpass_GEOMETRY,

		kSubpass_COUNT,
		kSubpass_MAX = kSubpass_COUNT - 1
	};

	enum
	{
		kAttachment_BACK, //is this ever needed?
		kAttachment_DEPTH,

		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};


	using AttachmentFormat = std::pair<VkFormat, VkImageUsageFlags>;

	std::array<AttachmentFormat, kAttachment_COUNT> fillFormatTable()
	{
		std::array<AttachmentFormat, kAttachment_COUNT> table = { };		
		return table;
	}

	std::array<AttachmentFormat, kAttachment_COUNT> attachmentFormatTable = fillFormatTable();

	ShadowRenderpass(SbVulkanBase& base, SbSwapchain& swapchain)
		: SbRenderpass(kSubpass_COUNT, kAttachment_COUNT)
	{
		attachmentFormatTable[kAttachment_DEPTH] =
			{ base.findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT };

		setUpSwapchainAttachments(swapchain);
		setUpRenderpass(swapchain);
	}

	void setUpSwapchainAttachments(SbSwapchain& swapchain) {
		swapchain.prepareAttachmentSets(kAttachment_COUNT);
		createAttachmentFor(swapchain, kAttachment_DEPTH);
	}

private:
	void setUpRenderpass(SbSwapchain& swapchain) override;
	void createAttachmentFor(SbSwapchain& swapchain, int attachmentIndex);

};

