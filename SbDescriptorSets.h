#pragma once

#include <vector>
#include "vulkan/vulkan.h"

#include "SbLayout.h"
#include "SbDescriptorPool.h"

/*
This class holds a number of descriptor sets (DS) that have been allocated from a pool. 
Each DS has a number of image/bufferinfos used to bind data to a binding in the DS
*/

class SbDescriptorSets
{	
public:
	enum BindingMode {
		eBindingMode_Shared,
		eBindingMode_Separate
	};

	struct SbImageInfo {
		VkSampler sampler;
		VkImageView* pView;
		VkImageLayout layout;
		BindingMode mode;
	};

	struct SbBufferInfo {
		VkBuffer* pBuffer;
		VkDeviceSize offset; 
		VkDeviceSize range;
		BindingMode mode;
	};
	
	
private:
	/*
	struct AllocatedDS{
		VkDescriptorSet allocatedDS; 
		std::vector<std::pair<const uint32_t, VkDescriptorImageInfo>> imageInfo;
		std::vector<std::pair<const uint32_t, VkDescriptorBufferInfo>> bufferInfo;
	};
	*/
	std::map<const uint32_t, const VkDescriptorSetLayoutBinding> bindings;
	std::map<const uint32_t, const SbImageInfo> imgInfo;
	std::map<const uint32_t, const SbBufferInfo> bufInfo;
	

	//const uint32_t DSCount;
	
	std::vector<VkDescriptorSetLayoutBinding> bindingsAsVector();

public:
	std::vector<VkDescriptorSet> allocatedDSs;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout DSLayout;


	//SbDescriptorSets(VkDevice device, std::vector<VkDescriptorSet> && DSs); //arg order
	SbDescriptorSets(const VkDevice & device, const uint32_t & descriptorSetCount);
	~SbDescriptorSets();

	const VkDevice device;
	
	void addImageBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbImageInfo & imageInfo);
	void addBufferBinding(const VkDescriptorSetLayoutBinding & newBinding, const SbBufferInfo & bufferInfo);



	void allocateDescriptorSets(const SbDescriptorPool & descriptorPool);
	void updateDescriptors();

	std::vector<VkDescriptorPoolSize> getRequiredPoolSizesForBindings();
	void createDSLayout();
	void createPipelineLayout(); //creates pipelinelayout using only this descriptor set




};


