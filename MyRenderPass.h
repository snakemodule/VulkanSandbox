#pragma once

#include "SbRenderpass.h"



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
		kAttachment_BACK,
		kAttachment_POSITION,
		kAttachment_NORMAL,
		kAttachment_ALBEDO,
		kAttachment_DEPTH,

		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};


	MyRenderPass(const SbVulkanBase& vkBase, SbSwapchain& swapchain)
		: SbRenderpass(vkBase, kSubpass_COUNT, kAttachment_COUNT, swapchain.getSize())
	{
		setUpRenderpass(swapchain);
	};

private:
    void setUpRenderpass(SbSwapchain& swapchain) override;

};

