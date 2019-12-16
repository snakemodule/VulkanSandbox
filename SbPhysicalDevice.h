#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <optional>

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class SbPhysicalDevice
{
public:
	VkPhysicalDevice device = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	SbPhysicalDevice(VkInstance instance, VkSurfaceKHR );
	~SbPhysicalDevice();


	//TODO should this class save keep a surface handle instead? pros cons?
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

private:
	VkSampleCountFlagBits getMaxUsableSampleCount();
	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);


};

