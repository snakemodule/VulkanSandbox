#pragma once

#include "vulkan/vulkan.h"

class SbBuffer
{
public:
	SbBuffer();
	~SbBuffer();

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;


};

