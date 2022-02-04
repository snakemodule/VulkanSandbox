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

	size_t dynamicAlignment = 0;
	size_t bufferSize = 0;
	
	size_t bufferLength = 0;

	SbUniformBuffer(SbVulkanBase & vkBase, size_t instanceCount, size_t bufferLength = 1)
	{
		buffers.resize(instanceCount);
		bufferMemory.resize(instanceCount);

		this->bufferLength = bufferLength;

		if (bufferLength > 1)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(vkBase.physicalDevice->device, &physicalDeviceProperties);

			size_t minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
			dynamicAlignment = sizeof(T);
			if (minUboAlignment > 0) {
				dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
			}

			bufferSize = bufferLength * dynamicAlignment;

			pUBOdata = static_cast<T*>(alignedAlloc(bufferSize, dynamicAlignment));
		}
		else
		{
			pUBOdata = new T();
			bufferSize = sizeof(T);
		}		
		assert(pUBOdata);
		for (size_t i = 0; i < instanceCount; i++) {
			vkBase.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				buffers[i], bufferMemory[i]);
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
				T & item = *(reinterpret_cast<T*>((uint64_t)pUBOdata + (i* offset * dynamicAlignment)));
				item = data;
			}
		}
		else
		{
			throw std::runtime_error("attempted to write outside uniform buffer allocation");
		}
	}

	T* getPtr(size_t offset = 0) 
	{
		return (reinterpret_cast<T*>((uint64_t)pUBOdata + (offset * dynamicAlignment)));
	}
	
	void copyDataToBufferMemory(SbVulkanBase & vkBase, uint32_t instance = 0)
	{
		void* data;
		vkMapMemory(vkBase.logicalDevice->device, bufferMemory[instance], 0, bufferSize, 0, &data);
		memcpy(data, pUBOdata, bufferSize);
		vkUnmapMemory(vkBase.logicalDevice->device, bufferMemory[instance]);
		
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
