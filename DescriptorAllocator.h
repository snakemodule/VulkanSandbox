#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>



/// <summary>
/// https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
/// </summary>
class DescriptorAllocator
{
public:

	struct PoolSizes {
		std::vector<std::pair<vk::DescriptorType, float>> sizes =
		{
			{ vk::DescriptorType::eSampler, 0.5f },
			{ vk::DescriptorType::eCombinedImageSampler, 4.f },
			{ vk::DescriptorType::eSampledImage , 4.f },
			{ vk::DescriptorType::eStorageImage, 1.f },
			{ vk::DescriptorType::eUniformTexelBuffer, 1.f },
			{ vk::DescriptorType::eStorageTexelBuffer, 1.f },
			{ vk::DescriptorType::eUniformBuffer, 2.f },
			{ vk::DescriptorType::eStorageBuffer, 2.f },
			{ vk::DescriptorType::eUniformBufferDynamic, 1.f },
			{ vk::DescriptorType::eStorageBufferDynamic, 1.f },
			{ vk::DescriptorType::eInputAttachment, 0.5f }
		};
	};

	void reset_pools();
	bool allocate(vk::DescriptorSetLayout layout, vk::DescriptorSet* set);

	void init(vk::Device newDevice);

	void cleanup();

	vk::Device device;
private:
	vk::DescriptorPool grab_pool();

	vk::DescriptorPool currentPool = vk::DescriptorPool();
	PoolSizes descriptorSizes;
	std::vector<vk::DescriptorPool> usedPools;
	std::vector<vk::DescriptorPool> freePools;
};

vk::DescriptorPool createPool(vk::Device device, const DescriptorAllocator::PoolSizes& poolSizes, int count, vk::DescriptorPoolCreateFlags flags);


