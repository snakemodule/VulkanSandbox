#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"

#include <vector>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class SbCommandPool;

#include "SbPhysicalDevice.h"
#include "SbLogicalDevice.h"
#include "SbCommandPool.h"


const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	//,"VK_LAYER_LUNARG_api_dump"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class SbVulkanBase
{
public:
	vk::Instance instance;	
	//VkDebugUtilsMessengerEXT debugMessenger;
	vk::DebugUtilsMessengerEXT debugMessenger;

	vk::SurfaceKHR surface;

	std::unique_ptr<SbPhysicalDevice> physicalDevice;
	std::unique_ptr<SbLogicalDevice> logicalDevice;
	std::unique_ptr<SbCommandPool> commandPool;

	//VkCommandPool commandPool;

	SbVulkanBase();
	~SbVulkanBase();

	void createInstance(bool validation);
	void setupDebugMessenger(bool validation);
	void createSurface(GLFWwindow * window);


	void pickPhysicalDevice();

	void createLogicalDevice();

	//temporary function location?
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	void createImage(VkExtent2D extent, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void submitCommandBuffers(std::vector<VkCommandBuffer> cmds, std::vector<VkSemaphore> waitSem, std::vector<VkSemaphore> finishedSem, VkFence inFlightFence);

private:
	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions(bool validation);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData);


	//VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugUtilsMessengerEXT * pDebugMessenger);



};

