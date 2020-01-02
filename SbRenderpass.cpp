#include "SbRenderpass.h"

#include <array>

#include "VulkanInitializers.hpp"

#include "Model.h" //TODO is this necessary, get vertex data some other way?

SbRenderpass::SbRenderpass(uint32_t subpassCount, uint32_t attachmentCount)
	: subpasses(subpassCount), attachments(attachmentCount)
{
}

SbRenderpass::~SbRenderpass()
{
}

void SbRenderpass::addAttachment(uint32_t attachmentIndex, VkAttachmentDescription desc)
{
	attachments[attachmentIndex] = desc;
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

void SbRenderpass::addSyncMasks(uint32_t subpassIndex, VkPipelineStageFlags pipelineMaskAsDst, VkPipelineStageFlags pipelineMaskAsSrc, VkAccessFlags accessMaskAsDst, VkAccessFlags accessMaskAsSrc)
{
	subpasses[subpassIndex].pipelineMaskAsDst = pipelineMaskAsDst;
	subpasses[subpassIndex].pipelineMaskAsSrc = pipelineMaskAsSrc;
	subpasses[subpassIndex].accessMaskAsDst = accessMaskAsDst;
	subpasses[subpassIndex].accessMaskAsSrc = accessMaskAsSrc;
}

void SbRenderpass::addDependency(uint32_t srcSubpassIndex, uint32_t dstSubpassIndex)
{
	dependencies.push_back(std::pair<uint32_t, uint32_t>(srcSubpassIndex, dstSubpassIndex));
}

void SbRenderpass::createRenderpass(SbSwapchain swapchain)
{
	std::vector<VkSubpassDependency> vkDependencies(dependencies.size());

	for (size_t i = 0; i < dependencies.size(); i++)
	{
		VkSubpassDependency dep;
		dep.srcSubpass = dependencies[i].first;
		dep.dstSubpass = dependencies[i].second;
		dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		if (dep.srcSubpass ==VK_SUBPASS_EXTERNAL)	{
			dep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dep.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		} else {
			dep.srcStageMask = subpasses[dep.srcSubpass].pipelineMaskAsSrc;
			dep.srcAccessMask = subpasses[dep.srcSubpass].accessMaskAsSrc;
		}

		if (dep.dstSubpass == VK_SUBPASS_EXTERNAL) {
			dep.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dep.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		} else {
			dep.dstStageMask = subpasses[dep.dstSubpass].pipelineMaskAsDst;
			dep.dstAccessMask = subpasses[dep.dstSubpass].accessMaskAsDst;
		}
		vkDependencies[i] = dep;
	}

	std::vector<VkSubpassDescription> subpassDescriptions(subpasses.size());

	for (size_t i = 0; i < subpasses.size(); i++)
	{
		VkSubpassDescription desc;
		desc.flags = 0;
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.colorAttachmentCount = subpasses[i].colorAttachments.size();
		desc.pColorAttachments = subpasses[i].colorAttachments.data();
		desc.pDepthStencilAttachment = (subpasses[i].depthStencilAttachment.attachment == VK_ATTACHMENT_UNUSED) ? nullptr :
			&subpasses[i].depthStencilAttachment;
		desc.inputAttachmentCount = subpasses[i].inputAttachments.size();
		desc.pInputAttachments = subpasses[i].inputAttachments.data();
		
		desc.pResolveAttachments = nullptr;
		desc.preserveAttachmentCount = 0;
		desc.pPreserveAttachments = nullptr;
		//subpassDescriptions.push_back(desc);
		subpassDescriptions[i] = desc;
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(vkDependencies.size());
	renderPassInfo.pDependencies = vkDependencies.data();

	if (vkCreateRenderPass(swapchain.logicalDevice.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

VkPipeline SbRenderpass::getSubpassPipeline(uint32_t subpassIndex)
{
	return subpasses[subpassIndex].pipeline.handle;
}

SbDescriptorSets & SbRenderpass::getSubpassDescriptorSets(uint32_t subpassIndex)
{
	return *subpasses[subpassIndex].descriptorSets;
}
