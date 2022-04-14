#include "SbRenderpass.h"

#include <array>

#include "VulkanInitializers.hpp"

#include "Model.h" //TODO is this necessary, get vertex data some other way?

SbRenderpass::SbRenderpass(uint32_t subpassCount, uint32_t attachmentCount)
	: subpasses(subpassCount), attachments(attachmentCount)
{
	//for (size_t i = 0; i < subpasses.size(); i++)
	//	subpasses[i].pipeline.subpassIndex(i);	
}

SbRenderpass::~SbRenderpass()
{
}


void SbRenderpass::addSwapchainAttachments(SbSwapchain & swapchain)
{
	for (size_t i = 0; i < swapchain.getAttachmentCount(); i++)
	{
		attachments[i] = swapchain.getAttachmentDescription(i);
	}
}

void SbRenderpass::addColorAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex) 
{
	VkAttachmentReference ref { attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	subpasses[subpassIndex].colorAttachments.push_back(ref);
}

void SbRenderpass::setDepthStencilAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex) 
{
	VkAttachmentReference ref{ attachmentIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	subpasses[subpassIndex].depthStencilAttachment = ref;
}

void SbRenderpass::addInputAttachmentRef(uint32_t subpassIndex, uint32_t attachmentIndex) 
{
	VkAttachmentReference ref{ attachmentIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	subpasses[subpassIndex].inputAttachments.push_back(ref);
}

void SbRenderpass::addDependency(VkSubpassDependency dep)
{
	dependencies.push_back(dep);
}

void SbRenderpass::createRenderpass(SbSwapchain& swapchain)
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
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(swapchain.logicalDevice.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}
