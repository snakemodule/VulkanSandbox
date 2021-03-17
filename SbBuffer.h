#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"

class SbVulkanBase;

/// <summary>
/// vulkan buffer backed by bound memory
/// </summary>
class SbBuffer
{
public:
	vk::DeviceSize bufferSize;
	vk::Buffer buffer;
	vk::DeviceMemory memory;

	SbBuffer(SbVulkanBase& base, vk::DeviceSize size, vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties);;

	void MapAndFill(vk::Device device, void* dataSrc, size_t bytes);

	void CopyBuffer(SbVulkanBase& base, SbBuffer dst);
	
	void Destroy(vk::Device device);
	

};

