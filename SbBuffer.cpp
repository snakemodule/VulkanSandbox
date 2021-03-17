#include "SbBuffer.h"

#include "SbVulkanBase.h"

SbBuffer::SbBuffer(SbVulkanBase& base, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	: bufferSize(size)
{
	auto createInfo = vk::BufferCreateInfo(vk::BufferCreateFlags(),
		bufferSize, usage, vk::SharingMode::eExclusive);
	buffer = base.getDevice().createBuffer(createInfo);

	vk::MemoryRequirements memRequirements =
		base.getDevice().getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memRequirements.size;

	vk::PhysicalDeviceMemoryProperties memProperties =
		base.getPhysicalDevice().getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((memRequirements.memoryTypeBits & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			allocInfo.memoryTypeIndex = i;
			break;
		}
	}

	memory = base.getDevice().allocateMemory(allocInfo);

	base.getDevice().bindBufferMemory(buffer, memory, 0);
}

void SbBuffer::MapAndFill(vk::Device device, void* dataSrc, size_t bytes)
{
	void* dataDst = device.mapMemory(memory, 0, bufferSize, vk::MemoryMapFlags());
	memcpy(dataDst, dataSrc, bytes);
	device.unmapMemory(memory);
}

void SbBuffer::CopyBuffer(SbVulkanBase& base, SbBuffer dst)
{
	vk::CommandBuffer commandBuffer = base.commandPool->beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = bufferSize;
	vkCmdCopyBuffer(commandBuffer, buffer, dst.buffer, 1, &copyRegion);

	base.commandPool->endSingleTimeCommands(commandBuffer);
}

void SbBuffer::Destroy(vk::Device device) {
	device.destroyBuffer(buffer);
	device.freeMemory(memory);
}
