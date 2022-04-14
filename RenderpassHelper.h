#pragma once

#include "vulkan/vulkan.h"
#include <vector>

#include "SbSwapchain.h"

#include "SbPipeline.h"

#include "SbFramebuffer.h"

class RenderpassHelper
{
public:

	struct Subpass {
		std::vector<VkAttachmentReference> inputAttachments;
		std::vector<VkAttachmentReference> colorAttachments;
		VkAttachmentReference depthStencilAttachment = { VK_ATTACHMENT_UNUSED , VK_IMAGE_LAYOUT_UNDEFINED };
	};

	std::vector<Subpass> subpasses;

	struct AttachmentInfo {
		VkImageUsageFlags usageMask;
		VkImageAspectFlags aspectMask;
	};
	std::vector<VkAttachmentDescription> desc;
	std::vector<AttachmentInfo> info;



	std::vector<VkSubpassDependency> dependencies = {};

	VkRenderPass renderPass;

	RenderpassHelper(uint32_t subpassCount, uint32_t attachmentCount);
	~RenderpassHelper();

	void colorAttachmentDesc(uint32_t attachmentIndex, VkFormat format);

	void depthAttachmentDesc(uint32_t attachmentIndex, VkFormat format);


	//virtual void setUpRenderpass(SbSwapchain& swapchain) = 0;


	
	void addAttachmentDescription(uint32_t attachmentIndex,
		VkFormat format,
		VkImageUsageFlags usage,
		VkImageUsageFlags additionalUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VkAttachmentLoadOp stencilLoad = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VkAttachmentStoreOp stencilStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);
	void addColorAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addInputAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void setDepthStencilAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addDependency(VkSubpassDependency dep);
	//void addAttachment(uint32_t attachmentIndex, VkAttachmentDescription desc);
	//void addSyncMasks(uint32_t subpassIndex,
	//	VkPipelineStageFlags stageMaskAsDst, VkPipelineStageFlags stageMaskAsSrc,
	//	VkAccessFlags accessMaskAsDst, VkAccessFlags accessMaskAsSrc);
	//void addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex);



	void createRenderpass(VkDevice);

	SbFramebuffer createFrameBuffer(SbVulkanBase* base, VkExtent2D extent);

	

	//VkPipeline getSubpassPipeline(uint32_t subpassIndex);

	//VkPipelineLayout getSubpassPipelineLayout(uint32_t subpassIndex);;
		

};



