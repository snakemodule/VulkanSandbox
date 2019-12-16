#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include <GLFW/glfw3.h>

#include "SbPhysicalDevice.h"
#include "SbLogicalDevice.h"


class SbSwapchain
{
public:
	SbSwapchain(SbPhysicalDevice & physDevice, SbLogicalDevice & logDevice);
	~SbSwapchain();



	SbPhysicalDevice & physDevice; 
	SbLogicalDevice & logDevice;	

	VkSwapchainKHR handle;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	void createSwapChain(VkSurfaceKHR surface, GLFWwindow * window);


	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities, GLFWwindow * window);

	void createImageViews(VkDevice device);

};

