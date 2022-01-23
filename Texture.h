#pragma once

#include "vulkan/vulkan.hpp"


struct Texture {
	vk::ImageView imageView;
	vk::Image image;
	VkDeviceMemory memory;
	vk::DeviceSize imageSize;
	VkImageCreateInfo imageCreateInfo;
	VkDescriptorImageInfo descriptorInfo;
};
