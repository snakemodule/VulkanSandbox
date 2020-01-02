#pragma once

#include "vulkan/vulkan.h"
#include <vector>

#include "SbSwapchain.h"

#include "SbDescriptorSets.h"

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

		std::unique_ptr<SbDescriptorSets> descriptorSets;
		//VkPipeline pipeline;
		SbPipeline pipeline;

	};

	std::vector<Subpass> subpasses;
	std::vector<VkAttachmentDescription> attachments;

	std::vector<std::pair<uint32_t, uint32_t>> dependencies;

	VkRenderPass renderPass;

	SbRenderpass(uint32_t subpassCount, uint32_t attachmentCount);
	~SbRenderpass();
	



	void addAttachment(uint32_t attachmentIndex, VkAttachmentDescription desc);
	void addColorAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void setDepthStencilAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addInputAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addSyncMasks(uint32_t subpassIndex,
		VkPipelineStageFlags stageMaskAsDst, VkPipelineStageFlags stageMaskAsSrc,
		VkAccessFlags accessMaskAsDst, VkAccessFlags accessMaskAsSrc);
	void addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex);
	void createRenderpass(SbSwapchain swapchain);

	VkPipeline getSubpassPipeline(uint32_t subpassIndex);
	SbDescriptorSets & getSubpassDescriptorSets(uint32_t subpassIndex);

};


