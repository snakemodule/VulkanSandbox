#pragma once

#include "vulkan/vulkan.h"
#include <vector>

#include "SbSwapchain.h"

#include "SbPipeline.h"

class SbRenderpass
{
public:

	struct Subpass {
		std::vector<VkAttachmentReference> inputAttachments;
		std::vector<VkAttachmentReference> colorAttachments;
		VkAttachmentReference depthStencilAttachment = { VK_ATTACHMENT_UNUSED , VK_IMAGE_LAYOUT_UNDEFINED };
	};

	std::vector<Subpass> subpasses;
	std::vector<VkAttachmentDescription> attachments;


	std::vector<VkSubpassDependency> dependencies = {};

	VkRenderPass renderPass;

	SbRenderpass(uint32_t subpassCount, uint32_t attachmentCount);
	~SbRenderpass();

	
	virtual void setUpRenderpass(SbSwapchain& swapchain) = 0;

	void addSwapchainAttachments(SbSwapchain & swapchain);
	void addColorAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addInputAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void setDepthStencilAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addDependency(VkSubpassDependency dep);
	//void addAttachment(uint32_t attachmentIndex, VkAttachmentDescription desc);
	//void addSyncMasks(uint32_t subpassIndex,
	//	VkPipelineStageFlags stageMaskAsDst, VkPipelineStageFlags stageMaskAsSrc,
	//	VkAccessFlags accessMaskAsDst, VkAccessFlags accessMaskAsSrc);
	//void addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex);



	void createRenderpass(SbSwapchain& swapchain);

	//VkPipeline getSubpassPipeline(uint32_t subpassIndex);

	//VkPipelineLayout getSubpassPipelineLayout(uint32_t subpassIndex);;

};


