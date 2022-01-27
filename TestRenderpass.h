#pragma once
#include "SbRenderpass.h"
class TestRenderpass :
    public SbRenderpass
{
public:
	enum
	{
		kSubpass_GBUF,
		kSubpass_COMPOSE,

		kSubpass_COUNT,
		kSubpass_MAX = kSubpass_COUNT - 1
	};

	enum
	{
		kAttachment_BACK, 
		//kAttachment_POSITION,
		//kAttachment_NORMAL,
		kAttachment_ALBEDO,
		//kAttachment_DEPTH,

		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};

	using AttachmentFormat = std::pair<VkFormat, VkImageUsageFlags>;
	std::array<AttachmentFormat, kAttachment_COUNT> attachmentFormatTable;

	TestRenderpass(SbVulkanBase& base, SbSwapchain& swapchain)
		: SbRenderpass(base, kSubpass_COUNT, kAttachment_COUNT, swapchain.getSize())
	{
		std::array<AttachmentFormat, kAttachment_COUNT>& table = attachmentFormatTable;
		//table[kAttachment_POSITION] = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		//table[kAttachment_NORMAL] = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		table[kAttachment_ALBEDO] = { VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		//table[kAttachment_DEPTH] = { base.findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT };

		swapchain.prepareAttachmentSets(kAttachment_COUNT);
		auto createAttachment = [&table, &swapchain](int attachmentIndex)
			{ swapchain.createAttachment(attachmentIndex, table[attachmentIndex].first, table[attachmentIndex].second); };
		//createAttachment(kAttachment_POSITION);
		//createAttachment(kAttachment_NORMAL);
		createAttachment(kAttachment_ALBEDO);
		//createAttachment(kAttachment_DEPTH);

		setUpRenderpass(swapchain);
	};

private:
	void setUpRenderpass(SbSwapchain& swapchain) override;
};

