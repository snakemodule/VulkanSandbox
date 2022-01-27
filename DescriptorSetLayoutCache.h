#pragma once

#include "vulkan/vulkan.hpp"

#include <unordered_map>


/// <summary>
/// https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
/// </summary>
class DescriptorSetLayoutCache
{
public:
	void init(vk::Device newDevice);
	void cleanup();

	vk::DescriptorSetLayout create_descriptor_layout(vk::DescriptorSetLayoutCreateInfo* info);

	struct DescriptorSetLayoutInfo {
		//good idea to turn this into a inlined array
		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorSetLayoutInfo& other) const;

		size_t hash() const;
	};



private:

	struct DescriptorSetLayoutHash {

		std::size_t operator()(const DescriptorSetLayoutInfo& k) const {
			return k.hash();
		}
	};

	std::unordered_map<DescriptorSetLayoutInfo, vk::DescriptorSetLayout, DescriptorSetLayoutHash> layoutCache;
	vk::Device device;
};

