#pragma once

#include "vulkan/vulkan.hpp"

#include "DescriptorSetLayoutCache.h"
#include "DescriptorAllocator.h"

#include "SbPipelineBuilder.h"

class DescriptorBuilder
{
public:
	static DescriptorBuilder begin(DescriptorSetLayoutCache* layoutCache, DescriptorAllocator* allocator);

	static DescriptorBuilder begin(SbPipelineBuilder* pipeline, DescriptorAllocator* allocator);

	DescriptorBuilder& bind_buffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo, vk::DescriptorType type, vk::ShaderStageFlags stageFlags);
	DescriptorBuilder& bind_image(uint32_t binding, vk::DescriptorImageInfo* imageInfo, vk::DescriptorType type, vk::ShaderStageFlags stageFlags);


	bool build(uint32_t set, vk::DescriptorSet& resultDesc, uint32_t instance);
	bool build(vk::DescriptorSet& set);
	
private:

	std::vector<vk::WriteDescriptorSet> writes;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	DescriptorSetLayoutCache* cache = nullptr;
	DescriptorAllocator* alloc = nullptr;
	SbPipelineBuilder* pipeline = nullptr;
};

