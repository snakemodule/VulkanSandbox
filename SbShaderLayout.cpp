#include "SbShaderLayout.h"

#include "spirv-cross/spirv_reflect.hpp"
#include "spirvloader.h"
#include "VulkanInitializers.hpp"
#include "VulkanHelperFunctions.hpp"
#include <iostream>
#include <algorithm>

namespace vkinit = vks::initializers;

VkShaderModule makeShaderModule(std::vector<uint32_t>& spirv_binary, VkDevice device)
{
	size_t size = spirv_binary.size() * sizeof(uint32_t);
	assert(size > 0);
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = size;
	moduleCreateInfo.pCode = spirv_binary.data();
	vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);
	return shaderModule;
}

VkPipelineShaderStageCreateInfo shaderStageCI(VkShaderModule shaderModule, VkShaderStageFlagBits stage, VkDevice device)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = shaderModule;
	shaderStage.pName = "main"; // todo : make param
	assert(shaderStage.module != VK_NULL_HANDLE);
	return shaderStage;
}

void SbShaderLayout::createDSLayout(vk::Device device, int set)
{
	auto& currentSet = sets[set];
	
	auto& bindingInfo = results.bindingInfo[set];
	for (auto& pair : currentSet.uniforms)
		bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pair.second.stageFlags, pair.first, 1));
	for (auto& pair : currentSet.imageSamplers)
		bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pair.second.stageFlags, pair.first, 1));
	for (auto& pair : currentSet.inputAttachments)
		bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pair.second.stageFlags, pair.first, 1));
	for (auto& pair : currentSet.storageBuffers)
		bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pair.second.stageFlags, pair.first, 1));

	std::sort(bindingInfo.begin(), bindingInfo.end(),
		[](VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs)
		-> bool { return lhs.binding < rhs.binding; });

	VkDescriptorSetLayoutCreateInfo DS_Layout_CI =
		vks::initializers::descriptorSetLayoutCreateInfo(bindingInfo);
	if (vkCreateDescriptorSetLayout(device, &DS_Layout_CI, nullptr, &results.setLayouts[set]) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void SbShaderLayout::createPipelineLayout(vk::Device device)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(results.setLayouts.data(), results.setLayouts.size());
	if (vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &results.pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void SbShaderLayout::parse(std::vector<uint32_t>& spirv_binary, VkShaderStageFlagBits shaderStage)
{
	spirv_cross::CompilerGLSL glsl(std::move(spirv_binary));

	// The SPIR-V is now parsed, and we can perform reflection on it.
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

	// Get all sampled images in the shader. (textures)
	for (auto& resource : resources.sampled_images)
	{
		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
		assert(set < 8 && set >= 0);
		if (sets.size() < set + 1)
			sets.resize(set + 1);

		auto& setImageSamplers = sets[set].imageSamplers;
		auto it = setImageSamplers.find(binding);
		if (it != setImageSamplers.end())
			it->second.stageFlags |= shaderStage;
		else
			setImageSamplers[binding] = { resource.name, (uint64_t)shaderStage };

		printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

	}

	for (auto& resource : resources.subpass_inputs) //attachments, use here or put early in renderpass construction
	{
		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
		unsigned attachment_index = glsl.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
		assert(set < 8 && set >= 0);
		if (sets.size() < set + 1)
			sets.resize(set + 1);

		auto& setSubpassInputs = sets[set].inputAttachments;
		auto it = setSubpassInputs.find(binding);
		if (it != setSubpassInputs.end())
			it->second.stageFlags |= shaderStage;
		else
			setSubpassInputs[binding] = { resource.name, (uint64_t)shaderStage, attachment_index };

		printf("subpass input %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
	}

	for (auto& resource : resources.uniform_buffers) {
		spirv_cross::SPIRType uniformType = glsl.get_type(resource.base_type_id);
		size_t size = glsl.get_declared_struct_size(uniformType);

		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
		assert(set < 8 && set >= 0);
		if (sets.size() < set + 1)
			sets.resize(set + 1);

		auto& setUniforms = sets[set].uniforms;
		auto it = setUniforms.find(binding);
		if (it != setUniforms.end())
			it->second.stageFlags |= shaderStage;
		else
			setUniforms[binding] = { resource.name, (uint64_t)shaderStage, (unsigned)size };
	}

	for (auto& resource : resources.storage_buffers) 
	{
		//spirv_cross::SPIRType uniformType = glsl.get_type(resource.base_type_id);

		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

		assert(set < 8 && set >= 0);
		if (sets.size() < set + 1)
			sets.resize(set + 1);

		auto& setBuffers = sets[set].storageBuffers;
		auto it = setBuffers.find(binding);
		if (it != setBuffers.end())
			it->second.stageFlags |= shaderStage;
		else
			setBuffers[binding] = { resource.name, (uint64_t)shaderStage };
	}

	for (set& s : sets)
	{
		assert(!(s.imageSamplers.empty() && s.inputAttachments.empty() && s.uniforms.empty() && s.storageBuffers.empty()));
	}
}

void SbShaderLayout::reflect(vk::Device device, std::string vert, std::string frag)
{
	std::vector<uint32_t> vert_binary = loadSpirvBinary(vert);
	std::vector<uint32_t> frag_binary = loadSpirvBinary(frag);
	assert(vert_binary.size() > 0);
	assert(frag_binary.size() > 0);
	auto vertModule = vks::helper::loadShader(vert.c_str(), device);//makeShaderModule(vert_binary, device);
	auto fragModule = vks::helper::loadShader(frag.c_str(), device);//makeShaderModule(frag_binary, device);
	auto vertCI = shaderStageCI(vertModule, VK_SHADER_STAGE_VERTEX_BIT, device);
	auto fragCI = shaderStageCI(fragModule, VK_SHADER_STAGE_FRAGMENT_BIT, device);
	results.shaderInfo.push_back(vertCI);
	results.shaderInfo.push_back(fragCI);
	parse(vert_binary, VK_SHADER_STAGE_VERTEX_BIT);
	parse(frag_binary, VK_SHADER_STAGE_FRAGMENT_BIT);

	results.bindingInfo.resize(sets.size());
	results.setLayouts.resize(sets.size());

	for (size_t i = 0; i < sets.size(); i++)
		createDSLayout(device, i);

	createPipelineLayout(device);
}

void SbShaderLayout::reflect(vk::Device device, std::string compute)
{
	std::vector<uint32_t> binary = loadSpirvBinary(compute);
	assert(binary.size() > 0);
	auto computeModule = vks::helper::loadShader(compute.c_str(), device);
	auto computeCI = shaderStageCI(computeModule, VK_SHADER_STAGE_COMPUTE_BIT, device);
	results.shaderInfo.push_back(computeCI);

	parse(binary, VK_SHADER_STAGE_COMPUTE_BIT);
	
	results.bindingInfo.resize(sets.size());
	results.setLayouts.resize(sets.size());

	for (size_t i = 0; i < sets.size(); i++)
		createDSLayout(device, i);

	createPipelineLayout(device);
}