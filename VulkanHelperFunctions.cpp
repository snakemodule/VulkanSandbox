#include "VulkanHelperFunctions.hpp"

VkShaderModule vks::helper::loadShader(const char* fileName, VkDevice device)
{
	std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

	if (is.is_open())
	{
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);
		char* shaderCode = new char[size];
		is.read(shaderCode, size);
		is.close();

		assert(size > 0);

		VkShaderModule shaderModule;
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;

		vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);

		delete[] shaderCode;

		return shaderModule;
	}
	else
	{
		std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << std::endl;
		return VK_NULL_HANDLE;
	}
}

VkPipelineShaderStageCreateInfo vks::helper::loadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice device)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = loadShader(fileName.c_str(), device);
	shaderStage.pName = "main"; // todo : make param
	assert(shaderStage.module != VK_NULL_HANDLE);
	return shaderStage;
}

//todo flytta till SbVulkanBase?
VkImageView vks::helper::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void vks::helper::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, 
	VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subresourceRange;

	//VkPipelineStageFlags sourceStage;
	//VkPipelineStageFlags destinationStage;


	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:		// Image layout is undefined (or does not matter)
		// Only valid as initial layout. 
		barrier.srcAccessMask = 0; //No flags required, listed only for completeness
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT; // Make sure host writes have been finished
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:// Image is a color attachment		
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// Make sure any writes to the color buffer have been finished
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:// Image is a depth/stencil attachment				
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;// Make sure any writes to the depth/stencil buffer have been finished
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:// Image is a transfer source		
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;// Make sure any reads from the image have been finished
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:// Image is a transfer destination
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;// Make sure any writes to the image have been finished
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:// Image is read by a shader
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Make sure any shader reads from the image have been finished
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:// Image will be used as a transfer destination
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;// Make sure any writes to the image have been finished
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:// Image will be used as a transfer source
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;// Make sure any reads from the image have been finished
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:// Image will be used as a color attachment
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// Make sure any writes to the color buffer have been finished
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:// Image layout will be used as a depth/stencil attachment
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;// Make sure any writes to depth/stencil buffer have been finished
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:// Image will be read in a shader (sampler, input attachment)		
		// Make sure any writes to the image have been finished
		if (barrier.srcAccessMask == 0) 
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;		
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}
	
	vkCmdPipelineBarrier(
		commandBuffer,
		srcStageMask, dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

}

void vks::helper::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
	VkImageAspectFlagBits aspect, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspect;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;

	transitionImageLayout(commandBuffer, image, oldLayout, newLayout, subresourceRange);
}