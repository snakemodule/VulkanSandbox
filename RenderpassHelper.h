#pragma once

#include "vulkan/vulkan.h"
#include <vector>

#include "SbSwapchain.h"

#include "SbPipeline.h"

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

	void addAttachmentDescription(uint32_t attachmentIndex, VkAttachmentDescription description);
		
	void addAttachmentDescription(uint32_t attachmentIndex,
		VkFormat format,
		VkImageUsageFlags usage,
		VkImageUsageFlags additionalUsage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, //todo not all attachments are input attachments
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VkAttachmentLoadOp stencilLoad = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VkAttachmentStoreOp stencilStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);
	void addColorAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addInputAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void setDepthStencilAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex);
	void addDependency(VkSubpassDependency dep);
	
	void createRenderpass(VkDevice);
	
};



