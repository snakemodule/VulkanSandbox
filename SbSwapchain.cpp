

#include "SbSwapchain.h"

#include <algorithm>

#include "VulkanHelperFunctions.hpp"

SbSwapchain::SbSwapchain(SbVulkanBase & base)
	:physicalDevice(*base.physicalDevice), logicalDevice(*base.logicalDevice), vulkanBase(base)
{
}


SbSwapchain::~SbSwapchain()
{
}

void SbSwapchain::createSwapChain(VkSurfaceKHR surface, GLFWwindow* window)
{
	SwapChainSupportDetails swapChainSupport = physicalDevice.querySwapChainSupport(physicalDevice.device, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	extent = chooseSwapExtent(swapChainSupport.capabilities, window);

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

	QueueFamilyIndices indices = physicalDevice.findQueueFamilies(physicalDevice.device, surface);
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

	if (vkCreateSwapchainKHR(logicalDevice.device, &createInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	createImageViews(); // here we set the size of the swapchain
		
}

//void SbSwapchain::prepareAttachmentSets(int attachmentCount)
//{
//	swapchainAttachmentSets = std::vector<SbSwapchain::SwapchainAttachment>(attachmentCount - 1, SbSwapchain::SwapchainAttachment(swapChainImages.size()));
//}

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

void SbSwapchain::createImageViews() 
{

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(logicalDevice.device, handle, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice.device, handle, &imageCount, swapChainImages.data());

	// Swap chain image color attachment
	// Will be transitioned to present layout
	auto & desc = swapchainAttachmentDescription;
	desc.flags = 0;
	desc.format = swapchainCI.imageFormat;
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	swapChainImageViews.resize(swapChainImages.size());

	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = vks::helper::createImageView(logicalDevice.device, swapChainImages[i], swapchainCI.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	
}

//void SbSwapchain::createFramebuffersForRenderpass(VkRenderPass renderpass) {
//	swapChainFramebuffers.resize(swapChainImageViews.size());
//
//	std::vector<VkImageView> attachmentViews (1+swapchainAttachmentSets.size());
//
//	VkFramebufferCreateInfo framebufferCI = {};
//	framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//	framebufferCI.renderPass = renderpass;
//	framebufferCI.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
//	framebufferCI.pAttachments = attachmentViews.data();
//	framebufferCI.width = swapchainCI.imageExtent.width;
//	framebufferCI.height = swapchainCI.imageExtent.height;
//	framebufferCI.layers = 1;
//
//	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
//
//		attachmentViews[0] = swapChainImageViews[i];
//		for (size_t j = 0; j < swapchainAttachmentSets.size(); j++)
//		{
//			attachmentViews[1+j] = swapchainAttachmentSets[j].view[i];
//		}
//
//		if (vkCreateFramebuffer(logicalDevice.device, &framebufferCI, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
//			throw std::runtime_error("failed to create framebuffer!");
//		}
//	}
//}

//todo use enum to configure common types?
/*
void SbSwapchain::createAttachment(
	uint32_t attachmentIndex,
	VkFormat format, 
	VkImageUsageFlags usage,
	VkImageUsageFlags additionalUsage,
	VkAttachmentLoadOp loadOp, 
	VkAttachmentStoreOp storeOp, 
	VkAttachmentLoadOp stencilLoad, 
	VkAttachmentStoreOp stencilStore) 
{
	assert(attachmentIndex > 0);
	--attachmentIndex;
	//assert(attachmentIndex < swapchainAttachmentSets.size());

	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;
	VkImageUsageFlags usageMask = (usage | additionalUsage);

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

	VkAttachmentDescription& colorAttachment = swapchainAttachmentSets[attachmentIndex].description;
	colorAttachment.flags = 0;
	colorAttachment.format = format;
	colorAttachment.samples = swapchainAttachmentDescription.samples;
	colorAttachment.loadOp = loadOp;
	colorAttachment.storeOp = storeOp;
	colorAttachment.stencilLoadOp = stencilLoad;
	colorAttachment.stencilStoreOp = stencilStore;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = imageLayout;


	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		auto& samples = swapchainAttachmentSets[attachmentIndex].description.samples;
		auto& image = swapchainAttachmentSets[attachmentIndex].image[i];
		auto& memory = swapchainAttachmentSets[attachmentIndex].mem[i];
		auto& view = swapchainAttachmentSets[attachmentIndex].view[i];
		vulkanBase.createImage(swapchainCI.imageExtent, 1, samples, format, VK_IMAGE_TILING_OPTIMAL,
			usageMask, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
		view = vks::helper::createImageView(logicalDevice.device, image, format, aspectMask, 1);
		vulkanBase.commandPool->transitionImageLayout(image, format, colorAttachment.initialLayout, imageLayout, 1);
	}
}*/

void SbSwapchain::createSyncObjects(const uint32_t MAX_FRAMES_IN_FLIGHT) {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(getSize(), VK_NULL_HANDLE);
	this->MAX_FRAMES_IN_FLIGHT = MAX_FRAMES_IN_FLIGHT;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(logicalDevice.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(logicalDevice.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(logicalDevice.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}



uint32_t SbSwapchain::acquireNextImage() {
	vkWaitForFences(logicalDevice.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice.device, this->handle,
		UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	/* //TODO recreate swapchain broken
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	*/

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(logicalDevice.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	return imageIndex;

}

void SbSwapchain::presentImage(uint32_t imageIndex, std::vector<VkSemaphore> waitSem) {
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = waitSem.size();
	presentInfo.pWaitSemaphores = waitSem.data();

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &this->handle;

	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(logicalDevice.presentQueue, &presentInfo);

	/* TODO HANDLE FRAME BUFFER RESIZE
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	*/
}

void SbSwapchain::updateFrameInFlightCounter() {
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void SbSwapchain::recreateSwapchain() {

}

VkSemaphore SbSwapchain::getImageAvailableSemaphore() {
	return imageAvailableSemaphores[currentFrame];
}

VkSemaphore SbSwapchain::getRenderFinishedSemaphore() {
	return renderFinishedSemaphores[currentFrame];
}

VkFence SbSwapchain::getInFlightFence() {
	return inFlightFences[currentFrame];
}

/*
std::vector<VkImageView> & SbSwapchain::getAttachmentViews(uint32_t index)
{
	assert(index >= 0);
	if (index==0)
	{
		return swapChainImageViews;
	}
	return swapchainAttachmentSets[index-1].view;
}

VkAttachmentDescription SbSwapchain::getAttachmentDescription(uint32_t index) {
	assert(index > 0);
	if (index == 0)
	{
		return swapchainAttachmentDescription;
	}
	return swapchainAttachmentSets[index - 1].description;
}


uint32_t SbSwapchain::getAttachmentCount()
{
	return swapchainAttachmentSets.size()+1;
}
*/

uint32_t SbSwapchain::getSize()
{
	return swapChainImages.size();
}
