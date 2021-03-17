#pragma once

#include "vulkan/vulkan.h"

#include "SbPhysicalDevice.h"

class SbLogicalDevice
{
public:
	SbLogicalDevice(SbPhysicalDevice & physicalDevice);
	~SbLogicalDevice();

	vk::Device device;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

	void createLogicalDevice(VkSurfaceKHR surface, bool validation, const std::vector<const char*> validationLayers);

	SbPhysicalDevice & physDevice;

};

