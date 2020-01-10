#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include <GLFW/glfw3.h>

#include "SbVulkanBase.h"


class SbSwapchain
{
	struct SwapchainAttachment {
		std::vector<VkImage> image;
		std::vector<VkDeviceMemory> mem;
		std::vector<VkImageView> view;
		//VkFormat format;
		//VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

		VkAttachmentDescription description;

		SwapchainAttachment(size_t size)
			: image(size), mem(size), view(size)
		{	}
	};

private:
	std::vector<SwapchainAttachment> swapchainAttachmentSets;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkAttachmentDescription swapchainAttachmentDescription; //TODO make use of attachment enums and integrate with renderpass creation

public:
	SbSwapchain(SbVulkanBase & base);
	~SbSwapchain();

	void createSwapChain(VkSurfaceKHR surface, GLFWwindow * window, uint32_t attachmentCount);


	SbVulkanBase & vulkanBase;
	SbPhysicalDevice & physicalDevice; 
	SbLogicalDevice & logicalDevice;	


	VkSwapchainCreateInfoKHR swapchainCI;
	VkSwapchainKHR handle;

	
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	uint32_t MAX_FRAMES_IN_FLIGHT; //TODO should really be const? will private do?

	

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities, GLFWwindow * window);

	void createImageViews();

	void createFramebuffers(VkRenderPass renderpass);

	void createAttachment(
		uint32_t attachmentIndex, 
		VkFormat format,
		VkImageUsageFlags usage,
		VkImageUsageFlags additionalUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VkAttachmentLoadOp stencilLoad = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VkAttachmentStoreOp stencilStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);

	void createSyncObjects(const uint32_t maxFramesInFlight);

	void recreateSwapchain();
	VkSemaphore getImageAvailableSemaphore();
	VkSemaphore getRenderFinishedSemaphore();
	VkFence getInFlightFence();
	uint32_t acquireNextImage();

	void presentImage(uint32_t imageIndex, std::vector<VkSemaphore> waitSem);

	void updateFrameInFlightCounter();


	VkSemaphore getImageAvailableSemaphore(uint32_t index);

	VkSemaphore getRenderFinishedSemaphore(uint32_t index);


	size_t getCurrentFrame();

	std::vector<VkImageView> & getAttachmentViews(uint32_t index);
	VkAttachmentDescription getAttachmentDescription(uint32_t index);

	uint32_t getSize();
	uint32_t getAttachmentCount();
};

