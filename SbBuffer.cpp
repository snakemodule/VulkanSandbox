#include "SbBuffer.h"

#include "SbVulkanBase.h"


SbBuffer::SbBuffer() 
{

}

/// <summary>
/// creates a devicelocal buffer and fills it with data via staging
/// </summary>
/// <param name="base"></param>
/// <param name="data"></param>
/// <param name="dataSize"></param>
/// <param name="usage"></param>
SbBuffer::SbBuffer(SbVulkanBase& base, void* data, uint32_t dataSize, vk::BufferUsageFlags usage)
	: SbBuffer(base, dataSize, usage | vk::BufferUsageFlagBits::eTransferDst, 
		vk::MemoryPropertyFlagBits::eDeviceLocal)
{
	//staged data fill
	SbBuffer stagingBuffer = SbBuffer(base, dataSize,
		vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBuffer.MapAndFill(base.getDevice(), data, dataSize);

	//VkCommandBuffer copyCmd = base->commandPool->beginSingleTimeCommands();
	//copy from staging to this
	stagingBuffer.CopyBuffer(base, *this);
}

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

void SbBuffer::CopyBuffer(SbVulkanBase& base, SbBuffer& dst)//todo rename CopyBufferSingle
{
	vk::CommandBuffer commandBuffer = base.commandPool->beginSingleTimeCommands();		
	CopyBuffer(commandBuffer, dst);
	base.commandPool->endSingleTimeCommands(commandBuffer);
}

void SbBuffer::CopyBuffer(VkCommandBuffer cmd, SbBuffer& dst) //todo rename CopyBufferRecord
{
	VkBufferCopy copyRegion = {};
	copyRegion.size = bufferSize;
	vkCmdCopyBuffer(cmd, buffer, dst.buffer, 1, &copyRegion);
}

void SbBuffer::Destroy(vk::Device device) {
	device.destroyBuffer(buffer);
	device.freeMemory(memory);
}
