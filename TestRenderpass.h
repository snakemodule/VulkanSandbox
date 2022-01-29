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
		kAttachment_POSITION,
		kAttachment_NORMAL,
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
		

		setUpRenderpass(swapchain);
	};

private:
	void setUpRenderpass(SbSwapchain& swapchain) override;
};

