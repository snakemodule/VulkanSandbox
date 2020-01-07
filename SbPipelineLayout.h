#pragma once

#include <vector>
#include "vulkan/vulkan.h"

#include "SbLayout.h"
#include "SbDescriptorPool.h"

#include "SbUniformBuffer.h"

/*
This class holds a number of descriptor sets (DS) that have been allocated from a pool. 
Each DS has a number of image/bufferinfos used to bind data to a binding in the DS
*/
class SbPipelineLayout
{	
public:
	//todo should be a property of the uniform buffer instead?
	enum SharingMode {
		eSharingMode_Shared,
		eSharingMode_Separate
	};

	struct SbImageInfo {
		const VkSampler sampler;
		const VkImageView * pView; //VkImageView* pView;
		const VkImageLayout layout;
		const SharingMode mode;
	};

	struct SbBufferInfo {
		const VkBuffer * pBuffer; //VkBuffer* pBuffer;
		const VkDeviceSize offset; 
		const VkDeviceSize range;
		const SharingMode mode;
	};
	
	//public for testing todo make private?
	std::map<const uint32_t, VkDescriptorSetLayoutBinding> bindings;
	std::map<const uint32_t, SbImageInfo> imgInfo;
	std::map<const uint32_t, SbBufferInfo> bufInfo;
private:	
	std::vector<VkDescriptorSetLayoutBinding> bindingsAsVector();

public:
	std::vector<VkDescriptorSet> allocatedDSs;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout DSLayout;

	SbPipelineLayout(const VkDevice & device, const uint32_t & descriptorSetCount);
	~SbPipelineLayout();

	const VkDevice device;
	
	void addImageBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbImageInfo & imageInfo);
	
	template <class T>
	void addBufferBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbUniformBuffer<T> & buffer) {
		//todo how to determine sharing mode?
		SharingMode mode = (buffer.buffers.size() == 1) ? eSharingMode_Shared : eSharingMode_Separate;
		SbBufferInfo info = { buffer.buffers.data(), 0, sizeof(T), mode };
		bindings.insert(std::make_pair(newBinding.binding, newBinding));
		bufInfo.insert(std::make_pair(newBinding.binding, info));
	}

	void allocateDescriptorSets(const SbDescriptorPool & descriptorPool);
	void updateDescriptors();

	std::vector<VkDescriptorPoolSize> getRequiredPoolSizesForBindings();
	void createDSLayout();
	void createPipelineLayout(); //creates pipelinelayout using only this descriptor set




};


