#pragma once

#include "VulkanInitializers.hpp"

#include <map>
#include <array>

#include <algorithm>

#include "algo.hpp"

/*
represents a descriptorset setup for a subpass
*/


//todo fix file name
//todo requirements of layout createion
class SbLayout
{

	//VkPipelineLayout pipelineLayout;


public:
	/*const VkDevice device;
	std::map<const uint32_t, VkDescriptorSetLayoutBinding> bindings;
	//public members? 
	VkDescriptorSetLayout DS_Layout;
	//std::vector<VkDescriptorSet> allocatedDSs; //not sure this should be here in the "layout" class
	
	
	SbLayout(SbLayout && other);
	SbLayout(VkDevice device);
	~SbLayout();

	void addBinding(VkDescriptorSetLayoutBinding newBinding);
	std::vector<VkDescriptorPoolSize> getRequiredPoolSizesForBindings();
	void createDSLayout();
	void createPipelineLayout(); //creates pipelinelayout using only this descriptor set*/

	
};

