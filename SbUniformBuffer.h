#pragma once

#include "SbBuffer.h"
#include <vector>

#include "SbVulkanBase.h"

template <class T>
class SbUniformBuffer
{
	T* pUBOdata = nullptr;

public:
	std::vector<VkBuffer> buffers = {};
	std::vector<VkDeviceMemory> bufferMemory = {};

	size_t dynamicOffset = 0;
	size_t bufferSize = 0;
	
	size_t bufferLength = 0;

	VkDevice device;

	SbUniformBuffer(SbVulkanBase & vkBase, size_t instanceCount, size_t bufferLength = 1, 
		VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
		device = vkBase.getDevice();

		buffers.resize(instanceCount);
		bufferMemory.resize(instanceCount);

		this->bufferLength = bufferLength;

		//if (bufferLength > 1)
		//{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(vkBase.getPhysicalDevice(), &physicalDeviceProperties);

			//temporary
			size_t minAlignment;
			if (bufferUsage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				minAlignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
			else if (bufferUsage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) 
				minAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
			
			//each element must be aligned to a 
			size_t ceil = ((sizeof(T) + minAlignment - 1) / minAlignment);
			dynamicOffset = ceil * minAlignment;

			//dynamicAlignment = sizeof(T);
			//if (minAlignment > 0) {
			//	dynamicAlignment = (dynamicAlignment + minAlignment - 1) & ~(minAlignment - 1);
			//}

			bufferSize = bufferLength * dynamicOffset;

			pUBOdata = static_cast<T*>(alignedAlloc(bufferSize, minAlignment));
		//}
		//else
		//{
		//	pUBOdata = new T();
		//	bufferSize = sizeof(T);
		//}		
		assert(pUBOdata);
		for (size_t i = 0; i < instanceCount; i++) {
			vkBase.createBuffer(bufferSize, bufferUsage, memoryProps, buffers[i], bufferMemory[i]);
		}
	}

	~SbUniformBuffer()
	{
		if (bufferSize > 0)
		{
			alignedFree(pUBOdata);
		}
		else
		{
			delete(pUBOdata);
		}
	}
	
	void writeBufferData(const T & data, size_t offset = 0, size_t count = 1)
	{
		if (offset < bufferLength)
		{
			for (size_t i = 0; i < count; i++)
			{
				T & item = *(reinterpret_cast<T*>((uint64_t)pUBOdata + (i* offset * dynamicOffset)));
				item = data;
			}
		}
		else
		{
			throw std::runtime_error("attempted to write outside uniform buffer allocation");
		}
	}

	T* data(size_t offset = 0) 
	{
		return (reinterpret_cast<T*>((uint64_t)pUBOdata + (offset * dynamicOffset)));
	}
	
	void writeToMappedMemory(uint32_t instance)
	{
		void* data;
		vkMapMemory(device, bufferMemory[instance], 0, bufferSize, 0, &data);
		memcpy(data, pUBOdata, bufferSize);
		vkUnmapMemory(device, bufferMemory[instance]);
		
	}

	void readFromMappedMemory(uint32_t instance)
	{
		void* data;
		vkMapMemory(device, bufferMemory[instance], 0, bufferSize, 0, &data);
		memcpy(pUBOdata, data, bufferSize);
		vkUnmapMemory(device, bufferMemory[instance]);
	}

	void* alignedAlloc(size_t size, size_t alignment)
	{
		void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
		data = _aligned_malloc(size, alignment);
#else 
		int res = posix_memalign(&data, alignment, size);
		if (res != 0)
			data = nullptr;
#endif
		return data;
	}

	void alignedFree(void* data)
	{
#if	defined(_MSC_VER) || defined(__MINGW32__)
		_aligned_free(data);
#else 
		free(data);
#endif
	}

	uint32_t getInstanceCount() {
		return buffers.size();
	}

};
