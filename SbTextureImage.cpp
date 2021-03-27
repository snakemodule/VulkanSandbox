#include "SbTextureImage.h"

#include "SbBuffer.h"
#include "SbImage.h"
#include "VulkanHelperFunctions.hpp"
#include "SbVulkanBase.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>



SbTextureImage::SbTextureImage(SbVulkanBase& base, std::string path)
{
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	imageSize = texWidth * texHeight * 4;
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	SbBuffer stagingBuffer = SbBuffer(base, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	stagingBuffer.MapAndFill(base.getDevice(), pixels, static_cast<size_t>((VkDeviceSize)imageSize));
	stbi_image_free(pixels);

	image = new SbImage(base, texWidth, texHeight, mipLevels,
		VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	base.commandPool->transitionImageLayout(image->img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	base.commandPool->copyBufferToImage(stagingBuffer.buffer, image->img,
		static_cast<uint32_t>(image->width), static_cast<uint32_t>(image->height));

	stagingBuffer.Destroy(base.getDevice());

	base.commandPool->generateMipmaps(image->img, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);

	textureImageView = vks::helper::createImageView(base.logicalDevice->device, image->img, 
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

void SbTextureImage::Destroy(vk::Device device) 
{
	image->Destroy(device);
	device.destroyImageView(textureImageView);
}

SbTextureImage::~SbTextureImage() 
{
	delete image;
}
