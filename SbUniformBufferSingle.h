#pragma once

#include "SbBuffer.h"
#include <vector>

#include "SbVulkanBase.h"

template <class T>
class SbUniformBufferSingle
{
	T* pUBOdata = nullptr;

public:
	VkBuffer buffer = {};
	VkDeviceMemory bufferMemory = {};

	size_t dynamicAlignment = 0;
	size_t bufferSize = 0;
	size_t bufferLength = 0;

	SbUniformBufferSingle()
	{		
	}

	void init(SbVulkanBase& vkBase, size_t bufferLength = 1) 
	{
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

		vkBase.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			buffer, bufferMemory);
	};

	~SbUniformBufferSingle()
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

	T& operator [](size_t i) const {
		T& item = *(reinterpret_cast<T*>((uint64_t)pUBOdata + (i * dynamicAlignment)));
		assert(i * dynamicAlignment < bufferLength);
		return item; 
	}

	//void writeBufferData(const T& data, size_t offset = 0, size_t count = 1)
	//{
	//	if (offset < bufferLength)
	//	{
	//		for (size_t i = 0; i < count; i++)
	//		{
	//			T& item = *(reinterpret_cast<T*>((uint64_t)pUBOdata + (i * offset * dynamicAlignment)));
	//			assert(i * offset * dynamicAlignment < bufferLength);
	//			item = data;
	//		}
	//	}		
	//}

	void copyToDeviceMemory(SbVulkanBase& vkBase)
	{
		void* data;
		vkMapMemory(vkBase.logicalDevice->device, bufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pUBOdata, bufferSize);
		vkUnmapMemory(vkBase.logicalDevice->device, bufferMemory);
	}

	void* alignedAlloc(size_t size, size_t alignment)
	{
		void* data = nullptr;
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

