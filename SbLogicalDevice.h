#pragma once

#include "vulkan/vulkan.h"

#include "SbPhysicalDevice.h"




class SbLogicalDevice
{
public:
	SbLogicalDevice(SbPhysicalDevice & physicalDevice);
	~SbLogicalDevice();

	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	void createLogicalDevice(VkSurfaceKHR surface, bool validation, const std::vector<const char*> validationLayers);

	SbPhysicalDevice & physDevice;

};

