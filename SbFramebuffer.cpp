#include "SbFramebuffer.h"

#include "VulkanHelperFunctions.hpp"

#include "SbVulkanBase.h"
#include "RenderpassHelper.h"

#include <cassert>


void SbFramebuffer::addAttachmentImage(uint32_t attachmentIndex, VkImageView imageView)
{
	views[attachmentIndex] = imageView;
}

void SbFramebuffer::createAttachmentImage(SbVulkanBase* base, RenderpassHelper* rp, uint32_t attachmentIndex)
{
	auto& image = images[attachmentIndex];
	auto& memory = imageMemory[attachmentIndex];
	auto& view = views[attachmentIndex];
	
	VkFormat format = rp->desc[attachmentIndex].format;
	base->createImage(extent, 1, rp->desc[attachmentIndex].samples, format, VK_IMAGE_TILING_OPTIMAL,
		rp->info[attachmentIndex].usageMask, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

	view = vks::helper::createImageView(base->getDevice(), image, format, rp->info[attachmentIndex].aspectMask, 1);

	base->commandPool->transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, rp->desc[attachmentIndex].finalLayout, 1);
	
}

void SbFramebuffer::createFramebuffer(VkDevice device)
{
	VkFramebufferCreateInfo ci = vks::initializers::framebufferCreateInfo();
	ci.flags = 0;
	ci.renderPass = renderpass;
	ci.attachmentCount = views.size();
	ci.pAttachments = views.data();
	ci.width = extent.width;
	ci.height = extent.height;
	ci.layers = 1;

	vkCreateFramebuffer(device, &ci, nullptr, &frameBuffer);
}

SbFramebuffer::SbFramebuffer(VkExtent2D extent, RenderpassHelper* rp)
	: images(rp->desc.size()),
	imageMemory(rp->desc.size()),
	views(rp->desc.size()),
	extent(extent),
	renderpass(rp->renderPass)
{	}

SbFramebuffer::SbFramebuffer()
{

}
