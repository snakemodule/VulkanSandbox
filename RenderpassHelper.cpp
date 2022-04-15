#include "RenderpassHelper.h"

#include <array>

#include "VulkanInitializers.hpp"
#include "VulkanHelperFunctions.hpp"



#include "Model.h" //TODO is this necessary, get vertex data some other way?

RenderpassHelper::RenderpassHelper(uint32_t subpassCount, uint32_t attachmentCount)
	: subpasses(subpassCount), info(attachmentCount), desc(attachmentCount)
{
	
}

RenderpassHelper::~RenderpassHelper()
{
}



void RenderpassHelper::colorAttachmentDesc(uint32_t attachmentIndex, VkFormat format) 
{
	addAttachmentDescription(attachmentIndex, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
}

void RenderpassHelper::depthAttachmentDesc(uint32_t attachmentIndex, VkFormat format)
{
	addAttachmentDescription(attachmentIndex, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void RenderpassHelper::addAttachmentDescription(uint32_t attachmentIndex, VkAttachmentDescription description) {
	desc[attachmentIndex] = description;
}

void RenderpassHelper::addAttachmentDescription(uint32_t attachmentIndex, 
	VkFormat format, 
	VkImageUsageFlags usage, 
	VkImageUsageFlags additionalUsage, 
	VkAttachmentLoadOp loadOp, 
	VkAttachmentStoreOp storeOp, 
	VkAttachmentLoadOp stencilLoad, 
	VkAttachmentStoreOp stencilStore)
{
	VkImageUsageFlags usageMask = (usage | additionalUsage);
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;
	if (usageMask & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else if (usageMask & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentDescription att;
	
	att.flags = 0;
	att.format = format;
	att.samples = VK_SAMPLE_COUNT_1_BIT; //todo hardcoded
	att.loadOp = loadOp;
	att.storeOp = storeOp;
	att.stencilLoadOp = stencilLoad;
	att.stencilStoreOp = stencilStore;
	att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	att.finalLayout = imageLayout;

	desc[attachmentIndex] = att;
	info[attachmentIndex].usageMask = usageMask;
	info[attachmentIndex].aspectMask = aspectMask;

};

void RenderpassHelper::addColorAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex)
{
	VkAttachmentReference ref{ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	subpasses[subpassIndex].colorAttachments.push_back(ref);
}

void RenderpassHelper::setDepthStencilAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex)
{
	VkAttachmentReference ref{ attachmentIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	subpasses[subpassIndex].depthStencilAttachment = ref;
}

void RenderpassHelper::addInputAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex)
{
	VkAttachmentReference ref{ attachmentIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	subpasses[subpassIndex].inputAttachments.push_back(ref);
}

void RenderpassHelper::addDependency(VkSubpassDependency dep)
{
	dependencies.push_back(dep);
}

void RenderpassHelper::createRenderpass(VkDevice device)
{
	std::vector<VkSubpassDescription> subpassDescriptions(subpasses.size());

	for (size_t i = 0; i < subpasses.size(); i++)
	{
		VkSubpassDescription desc;
		desc.flags = 0;
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.colorAttachmentCount = subpasses[i].colorAttachments.size();
		desc.pColorAttachments = subpasses[i].colorAttachments.data();
		desc.pDepthStencilAttachment = (subpasses[i].depthStencilAttachment.attachment == VK_ATTACHMENT_UNUSED) ?
			nullptr : &subpasses[i].depthStencilAttachment;
		desc.inputAttachmentCount = subpasses[i].inputAttachments.size();
		desc.pInputAttachments = subpasses[i].inputAttachments.data();

		desc.pResolveAttachments = nullptr;
		desc.preserveAttachmentCount = 0;
		desc.pPreserveAttachments = nullptr;
		subpassDescriptions[i] = desc;
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(info.size());
	renderPassInfo.pAttachments = desc.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}


