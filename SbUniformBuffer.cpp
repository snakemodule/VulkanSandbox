/*
#include "SbUniformBuffer.h"

#include <assert.h>     

template <class T>
SbUniformBuffer<T>::SbUniformBuffer(SbVulkanBase & vkBase, size_t instanceCount, size_t bufferSize)
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
	
	for (size_t i = 0; i < instanceCount; i++) {
		vkBase.createBuffer(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			realbuffers[i], buffers[i].bufferMemory);
	}
}

template <class T>
SbUniformBuffer<T>::~SbUniformBuffer()
{
	alignedFree(UBOdata);
}

template<class T>
void SbUniformBuffer<T>::writeBufferData(const T & data, size_t offset)
{
	if (offset < buffers.size())
	{
		T & UBO = *(static_cast<T*>((uint64_t)UBOdata + (offset * dynamicAlignment)));
		UBO = data;
	}
	else 
	{
		throw std::runtime_error("attempted to write outside uniform buffer allocation");
	}
}

template<class T>
void SbUniformBuffer<T>::copyBufferDataToMemory(SbVulkanBase & vkBase)
{
	for (size_t i = 0; i < buffers.size(); i++)
	{
		void* data;
		vkMapMemory(vkBase.logicalDevice->device, buffers[i].bufferMemory, 0, dynamicBufferSize, 0, &data);
		memcpy(data, UBOdata, dynamicBufferSize);
		vkUnmapMemory(vkBase.logicalDevice->device, buffers[i].bufferMemory);
	}
}

template<class T>
void* SbUniformBuffer<T>::alignedAlloc(size_t size, size_t alignment)
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

template<class T>
void SbUniformBuffer<T>::alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else 
	free(data);
#endif
}

*/