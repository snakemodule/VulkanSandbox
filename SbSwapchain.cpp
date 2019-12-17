

#include "SbSwapchain.h"

#include <algorithm>

#include "VulkanHelperFunctions.hpp"

SbSwapchain::SbSwapchain(SbPhysicalDevice & physDevice, SbLogicalDevice & logDevice)
	: logDevice(logDevice), physDevice(physDevice)
{
}


SbSwapchain::~SbSwapchain()
{
}

void SbSwapchain::createSwapChain(VkSurfaceKHR surface, GLFWwindow* window)
{
	SwapChainSupportDetails swapChainSupport = physDevice.querySwapChainSupport(physDevice.device, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = physDevice.findQueueFamilies(physDevice.device, surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	swapchainCI = createInfo;

	if (vkCreateSwapchainKHR(logDevice.device, &createInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	

}


VkSurfaceFormatKHR SbSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SbSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D SbSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void SbSwapchain::createImageViews(VkDevice device) {

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(logDevice.device, handle, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logDevice.device, handle, &imageCount, swapChainImages.data());

	// Swap chain image color attachment
	// Will be transitioned to present layout
	auto & desc = swapchainAttachmentDescription;
	desc.format = swapchainCI.imageFormat;
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	swapChainImageViews.resize(swapChainImages.size());

	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = vks::helper::createImageView(device, swapChainImages[i], desc.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	const SbSwapchain::SwapchainAttachment as(swapChainImages.size());
	swapchainAttachmentSets = std::vector<SbSwapchain::SwapchainAttachment>(SbSwapchain::attachmentIndex::eSetIndex_COUNT, as);
}

void SbSwapchain::createFramebuffers(VkRenderPass renderpass) {
	swapChainFramebuffers.resize(swapChainImageViews.size());


	std::vector<VkImageView> attachmentViews (1+swapchainAttachmentSets.size());

	VkFramebufferCreateInfo framebufferCI = {};
	framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCI.renderPass = renderpass;
	framebufferCI.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
	framebufferCI.pAttachments = attachmentViews.data();
	framebufferCI.width = swapchainCI.imageExtent.width;
	framebufferCI.height = swapchainCI.imageExtent.height;
	framebufferCI.layers = 1;

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {

		attachmentViews[0] = swapChainImageViews[i];
		for (size_t j = 0; j < swapchainAttachmentSets.size(); j++)
		{
			attachmentViews[1+j] = swapchainAttachmentSets[j].view[i];
		}
		//attachmentViews[kAttachment_COLOR] = swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Color].view[i]; //attachments[i].color.view;
		//attachmentViews[kAttachment_DEPTH] = swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Depth].view[i];

		if (vkCreateFramebuffer(logDevice.device, &framebufferCI, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}