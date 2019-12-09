#include "SbLayout.h"

#include <iterator>

/*

SbLayout::SbLayout(SbLayout && other)
	: device(other.device), bindings(std::move(other.bindings))
{
	DS_Layout = other.DS_Layout;
	pipelineLayout = other.pipelineLayout;
}

SbLayout::SbLayout(VkDevice device)
	:device(device)
{

}

SbLayout::~SbLayout()
{

}


void SbLayout::addBinding(VkDescriptorSetLayoutBinding newBinding)
{
	bindings.insert(std::make_pair(newBinding.binding, newBinding));
}

std::vector<VkDescriptorPoolSize> SbLayout::getRequiredPoolSizesForBindings()
{ 
	std::vector<VkDescriptorPoolSize> resultVector;
	std::for_each(bindings.begin(), bindings.end(), [&](std::pair<const uint32_t, VkDescriptorSetLayoutBinding> & KVpair) {
		auto & binding = KVpair.second;
		VkDescriptorPoolSize ps = { binding.descriptorType, binding.binding };
		resultVector.push_back(ps);
	});
	return resultVector;
}

std::vector<VkDescriptorSetLayoutBinding> SbLayout::getBindings()
{
	std::vector<VkDescriptorSetLayoutBinding> resultVector;
	for (auto item : bindings) {
		auto & binding = item.second;
		resultVector.push_back(binding);
	}
	return resultVector;
}


void SbLayout::createDSLayout()
{
	VkDescriptorSetLayoutCreateInfo DS_Layout_CI = vks::initializers::descriptorSetLayoutCreateInfo(getBindings());
	if (vkCreateDescriptorSetLayout(device, &DS_Layout_CI, nullptr, &DS_Layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void SbLayout::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&DS_Layout);
	if (vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
*/