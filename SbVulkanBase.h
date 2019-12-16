#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	//,"VK_LAYER_LUNARG_api_dump"
};

class SbVulkanBase
{
public:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	SbVulkanBase();
	~SbVulkanBase();

	void createInstance(bool validation);

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions(bool validation);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData);

	void setupDebugMessenger(bool validation);

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugUtilsMessengerEXT * pDebugMessenger);

	void createSurface(GLFWwindow * window);



};

