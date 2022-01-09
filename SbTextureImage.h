#pragma once

#include <string>
#include "vulkan/vulkan.h"
#include "vulkan/vulkan.hpp"

class SbImage;
class SbVulkanBase;

class SbTextureImage
{
public:
	vk::ImageView textureImageView;
	SbImage* image;

	int texWidth, texHeight, texChannels;
	vk::DeviceSize imageSize;
	uint32_t mipLevels;
	//VkImageLayout layout;

	
	SbTextureImage(SbVulkanBase& base, std::string path);

	void Destroy(vk::Device device);

	~SbTextureImage();
};

