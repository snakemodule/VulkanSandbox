#include "TestRenderpass.h"



void TestRenderpass::setUpRenderpass(SbSwapchain& swapchain)
{
	addSwapchainAttachments(swapchain);

	addColorAttachmentRef(kSubpass_GBUF, kAttachment_BACK);
	//addColorAttachmentRef(kSubpass_GBUF, kAttachment_POSITION);
	//addColorAttachmentRef(kSubpass_GBUF, kAttachment_NORMAL);
	addColorAttachmentRef(kSubpass_GBUF, kAttachment_ALBEDO);
	//setDepthStencilAttachmentRef(kSubpass_GBUF, kAttachment_DEPTH);

	addColorAttachmentRef(kSubpass_COMPOSE, kAttachment_BACK);
	//addInputAttachmentRef(kSubpass_COMPOSE, kAttachment_POSITION);
	//addInputAttachmentRef(kSubpass_COMPOSE, kAttachment_NORMAL);
	addInputAttachmentRef(kSubpass_COMPOSE, kAttachment_ALBEDO);
	//setDepthStencilAttachmentRef(kSubpass_COMPOSE, kAttachment_DEPTH);
		
	std::array<VkSubpassDependency, 3> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This dependency transitions the input attachment from color attachment to shader read
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = 1;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	dependencies[2].srcSubpass = 1;
	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	addDependency(dependencies[0]);
	addDependency(dependencies[1]);
	addDependency(dependencies[2]);

	createRenderpass(swapchain);
}
