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
		VkPipelineStageFlags pipelineMaskAsDst;
		VkPipelineStageFlags pipelineMaskAsSrc;
		VkAccessFlags accessMaskAsDst;
		VkAccessFlags accessMaskAsSrc;

		//SbPipeline pipeline;

		std::vector<SbPipeline> pipelines;

		Subpass(const SbVulkanBase & vkBase, const uint32_t & swapchainSize);

	};



	std::vector<Subpass> subpasses;
	std::vector<VkAttachmentDescription> attachments;

	std::vector<std::pair<uint32_t, uint32_t>> dependencies = {};

	std::vector<VkSubpassDependency> premadeDependencies = {};

	VkRenderPass renderPass;

	SbRenderpass(const SbVulkanBase & vkBase, uint32_t subpassCount, uint32_t attachmentCount, uint32_t swapchainSize);
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

	VkPipeline getSubpassPipeline(uint32_t subpassIndex, uint32_t pipeline = 0);

	VkPipelineLayout getSubpassPipelineLayout(uint32_t subpassIndex, uint32_t pipelineIndex = 0);

};


