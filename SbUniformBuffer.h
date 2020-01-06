#pragma once

#include "SbBuffer.h"
#include <vector>

#include "SbVulkanBase.h"

template <class T>
class SbUniformBuffer
{
	T* UBOdata = nullptr;

	std::vector<SbBuffer> buffers = {};

public:
	//TODO DELETE THIS
	std::vector<VkBuffer> realbuffers = {};
	size_t dynamicAlignment = 0;
	size_t dynamicBufferSize = 0;

	/*
	SbUniformBuffer(SbVulkanBase & vkBase, size_t instanceCount, size_t bufferSize = 1);
	~SbUniformBuffer();

	void writeBufferData(const T & data, size_t offset = 0);

	void copyBufferDataToMemory(SbVulkanBase & vkBase);

// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
	void* alignedAlloc(size_t size, size_t alignment);

	void alignedFree(void* data);
	*/

	SbUniformBuffer(SbVulkanBase & vkBase, size_t instanceCount, size_t bufferSize)
	{

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(vkBase.physicalDevice->device, &physicalDeviceProperties);

		size_t minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
		dynamicAlignment = sizeof(T);
		if (minUboAlignment > 0) {
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}

		dynamicBufferSize = bufferSize * dynamicAlignment;

		UBOdata = static_cast<T*>(alignedAlloc(dynamicBufferSize, dynamicAlignment));
		assert(UBOdata);

		buffers.resize(instanceCount);
		realbuffers.resize(instanceCount);

		for (size_t i = 0; i < instanceCount; i++) {
			vkBase.createBuffer(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				realbuffers[i], buffers[i].bufferMemory);
		}
	}

	~SbUniformBuffer()
	{
		alignedFree(UBOdata);
	}
	
	void writeBufferData(const T & data, size_t offset)
	{
		if (offset < buffers.size())
		{
			T & UBO = *(reinterpret_cast<T*>((uint64_t)UBOdata + (offset * dynamicAlignment)));
			UBO = data;
		}
		else
		{
			throw std::runtime_error("attempted to write outside uniform buffer allocation");
		}
	}
	
	void copyBufferDataToMemory(SbVulkanBase & vkBase)
	{
		for (size_t i = 0; i < buffers.size(); i++)
		{
			void* data;
			vkMapMemory(vkBase.logicalDevice->device, buffers[i].bufferMemory, 0, dynamicBufferSize, 0, &data);
			memcpy(data, UBOdata, dynamicBufferSize);
			vkUnmapMemory(vkBase.logicalDevice->device, buffers[i].bufferMemory);
		}
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

};