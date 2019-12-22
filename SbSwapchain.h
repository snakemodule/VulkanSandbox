#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include <GLFW/glfw3.h>

#include "SbVulkanBase.h"


class SbSwapchain
{
public:
	SbSwapchain(SbVulkanBase & base);
	~SbSwapchain();


	SbVulkanBase & vulkanBase;
	SbPhysicalDevice & physicalDevice; 
	SbLogicalDevice & logicalDevice;	


	VkSwapchainCreateInfoKHR swapchainCI;
	VkSwapchainKHR handle;

	//VkExtent2D swapChainExtent;

	//VkFormat swapChainImageFormat;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkAttachmentDescription swapchainAttachmentDescription; //TODO make use of attachment enums and integrate with renderpass creation

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

	
	std::vector<SwapchainAttachment> swapchainAttachmentSets;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	void createSwapChain(VkSurfaceKHR surface, GLFWwindow * window);


	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities, GLFWwindow * window);

	void createImageViews(VkDevice device);

	void setAttachmentCount(uint32_t count);

	void createFramebuffers(VkRenderPass renderpass);

	void createAttachment(
		uint32_t attachmentIndex, 
		VkFormat format,
		VkImageLayout finalLayout,
		VkImageUsageFlags usageBits,
		VkImageAspectFlags aspectBits,
		VkImageLayout initLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VkAttachmentLoadOp stencilLoad = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VkAttachmentStoreOp stencilStore = VK_ATTACHMENT_STORE_OP_DONT_CARE);

};

