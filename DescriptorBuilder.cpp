#include "DescriptorBuilder.h"

DescriptorBuilder DescriptorBuilder::begin(DescriptorSetLayoutCache* layoutCache, DescriptorAllocator* allocator) {

	DescriptorBuilder builder;

	builder.cache = layoutCache;
	builder.alloc = allocator;
	return builder;
}



DescriptorBuilder DescriptorBuilder::begin(SbPipelineBuilder* pipeline, DescriptorAllocator* allocator)
{
	DescriptorBuilder builder;

	builder.pipeline = pipeline;
	builder.alloc = allocator;




	return builder;
}

DescriptorBuilder& DescriptorBuilder::bind_buffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo, vk::DescriptorType type, vk::ShaderStageFlags stageFlags)
{
	//create the descriptor binding for the layout
	vk::DescriptorSetLayoutBinding newBinding{};

	newBinding.descriptorCount = 1;
	newBinding.descriptorType = type;
	newBinding.pImmutableSamplers = nullptr;
	newBinding.stageFlags = stageFlags;
	newBinding.binding = binding;

	bindings.push_back(newBinding);

	//create the descriptor write
	vk::WriteDescriptorSet newWrite{};
	newWrite.pNext = nullptr;

	newWrite.descriptorCount = 1;
	newWrite.descriptorType = type;
	newWrite.pBufferInfo = bufferInfo;
	newWrite.dstBinding = binding;

	writes.push_back(newWrite);
	return *this;
}

DescriptorBuilder& DescriptorBuilder::bind_image(uint32_t binding, vk::DescriptorImageInfo* imageInfo, vk::DescriptorType type, vk::ShaderStageFlags stageFlags)
{
	//create the descriptor binding for the layout
	vk::DescriptorSetLayoutBinding newBinding{};
	newBinding.descriptorCount = 1;
	newBinding.descriptorType = type;
	newBinding.pImmutableSamplers = nullptr;
	newBinding.stageFlags = stageFlags;
	newBinding.binding = binding;

	bindings.push_back(newBinding);

	//create the descriptor write
	vk::WriteDescriptorSet newWrite{};
	newWrite.pNext = nullptr;
	newWrite.descriptorCount = 1;
	newWrite.descriptorType = type;
	newWrite.pImageInfo = imageInfo;
	newWrite.dstBinding = binding;

	writes.push_back(newWrite);
	return *this;
}

bool DescriptorBuilder::build(uint32_t set, vk::DescriptorSet& resultDesc, uint32_t instance)
{
	vk::DescriptorSetLayout DSL = pipeline->DSL[set];

	for (uint64_t binding = 0; binding < pipeline->bindings[set].size(); binding++)
	{
		vk::DescriptorSetLayoutBinding& DSL_binding = pipeline->bindings[set][binding];
		if (DSL_binding.descriptorType == vk::DescriptorType::eInputAttachment)
		{
			//create the descriptor write
			vk::WriteDescriptorSet w{};
			w.pNext = nullptr;
			//w.dstSet = pipeline->DSL[set];
			w.dstBinding = DSL_binding.binding;
			w.descriptorCount = 1;
			w.descriptorType = vk::DescriptorType::eInputAttachment;

			uint64_t set_binding_combo = ((uint64_t)set << 32) | (uint64_t)DSL_binding.binding;
			auto& info_instances = pipeline->input_attachments_descriptor_info[set_binding_combo];
			//for (size_t i = 0; i < info_instances.size(); i++)
			//{
			w.pImageInfo = &info_instances[instance];
			writes.push_back(w);
			//}
		}
	}

	//allocate descriptor
	bool success = alloc->allocate(DSL, &resultDesc);
	if (!success) { return false; };

	//write descriptor
	for (vk::WriteDescriptorSet& w : writes) {
		w.dstSet = resultDesc;
	}

	alloc->device.updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);

	return true;
}
