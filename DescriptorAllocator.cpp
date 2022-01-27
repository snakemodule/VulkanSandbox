#include "DescriptorAllocator.h"

void DescriptorAllocator::reset_pools() {
	//reset all used pools and add them to the free pools
	for (auto p : usedPools) {
		vkResetDescriptorPool(device, p, 0);
		freePools.push_back(p);
	}

	//clear the used pools, since we've put them all in the free pools
	usedPools.clear();

	//reset the current pool handle back to null
	currentPool = vk::DescriptorPool();
}


bool DescriptorAllocator::allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout layout)
{
	//initialize the currentPool handle if it's null
	if (currentPool == vk::DescriptorPool()) {

		currentPool = grab_pool();
		usedPools.push_back(currentPool);
	}

	vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pSetLayouts = &layout;
	allocInfo.descriptorPool = currentPool;
	allocInfo.descriptorSetCount = 1;

	//try to allocate the descriptor set
	
	vk::Result allocResult = device.allocateDescriptorSets(&allocInfo, set);
	bool needReallocate = false;

	switch (allocResult) {
	case vk::Result::eSuccess:
		//all good, return
		return true;
	case vk::Result::eErrorFragmentedPool:
	case vk::Result::eErrorOutOfPoolMemory:
		//reallocate pool
		needReallocate = true;
		break;
	default:
		throw std::runtime_error("unrecoverable error");
		return false;
	}

	if (needReallocate) {
		//allocate a new pool and retry
		currentPool = grab_pool();
		usedPools.push_back(currentPool);

		allocResult = device.allocateDescriptorSets(&allocInfo, set);

		//if it still fails then we have big issues
		if (allocResult == vk::Result::eSuccess) {
			return true;
		}
	}
	throw std::runtime_error("failed to allocate descriptor from freshly grabbed pool");
	return false;
}


void DescriptorAllocator::init(vk::Device newDevice)
{
	device = newDevice;
}

void DescriptorAllocator::cleanup()
{
	//delete every pool held
	for (auto p : freePools)
	{
		vkDestroyDescriptorPool(device, p, nullptr);
	}
	for (auto p : usedPools)
	{
		vkDestroyDescriptorPool(device, p, nullptr);
	}
}

vk::DescriptorPool DescriptorAllocator::grab_pool()
{
	//there are reusable pools availible
	if (freePools.size() > 0)
	{
		//grab pool from the back of the vector and remove it from there.
		vk::DescriptorPool pool = freePools.back();
		freePools.pop_back();
		return pool;
	}
	else
	{
		//no pools availible, so create a new one
		return createPool(device, descriptorSizes, 1000, vk::DescriptorPoolCreateFlags());
	}
}

vk::DescriptorPool createPool(vk::Device device, const DescriptorAllocator::PoolSizes& poolSizes, int count, vk::DescriptorPoolCreateFlags flags)
{
	std::vector<vk::DescriptorPoolSize> sizes;
	sizes.reserve(poolSizes.sizes.size());
	for (auto sz : poolSizes.sizes) {
		sizes.push_back({ sz.first, uint32_t(sz.second * count) });
	}
	vk::DescriptorPoolCreateInfo pool_info;
	pool_info.flags = flags;
	pool_info.maxSets = count;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	vk::DescriptorPool descriptorPool = device.createDescriptorPool(pool_info);

	return descriptorPool;
}
