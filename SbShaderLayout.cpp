#include "SbShaderLayout.h"

#include "spirv-cross/spirv_reflect.hpp"
#include "spirvloader.h"
#include "VulkanInitializers.hpp"
#include "VulkanHelperFunctions.h"
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
	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	return shaderStage;
}

SbShaderLayout::SbSetLayout SbShaderLayout::createDSLayout(vk::Device device, int set)
{
	auto& currentSet = sets[set];
	auto& imageSamplers = currentSet.imageSamplers;
	auto& uniforms = currentSet.uniforms;
	auto& inputAttachments = currentSet.inputAttachments;

	SbSetLayout layout;

	for (auto& pair : uniforms)
		layout.bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pair.second.stageFlags, pair.first));
	for (auto& pair : imageSamplers)
		layout.bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pair.second.stageFlags, pair.first));
	for (auto& pair : inputAttachments)
		layout.bindingInfo.push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pair.second.stageFlags, pair.first));

	//std::sort(layout.bindingInfo.begin(), layout.bindingInfo.end(),
	//	[](VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs)
	//	-> bool { return lhs.binding < rhs.binding; });

	//vk::DescriptorSetLayoutCreateInfo DS_Layout_CI = vk::DescriptorSetLayoutCreateInfo{}
	//	.setBindingCount(2)
	//	.setPBindings(layout.bindingInfo.data());
	//
	//
	//descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//descriptorSetLayoutCreateInfo.pBindings = bindings.data();
	//descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());

	VkDescriptorSetLayoutCreateInfo DS_Layout_CI =
		vks::initializers::descriptorSetLayoutCreateInfo(layout.bindingInfo);
	if (vkCreateDescriptorSetLayout(device, &DS_Layout_CI, nullptr, &layout.layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
	return layout;
}

/*
VkPipelineLayout SbShaderLayout::createPipelineLayout(vk::Device device)
{
	std::vector<VkDescriptorSetLayout> setLayouts = std::vector<VkDescriptorSetLayout>(sbSetLayouts.size());
	for (size_t i = 0; i < setLayouts.size(); i++)
		setLayouts[i] = sbSetLayouts[i].layout;

	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(setLayouts.data(),setLayouts.size());
	VkPipelineLayout pl;
	if (vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pl) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	return pl;
}
*/

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

		printf("Uniform buffer %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
	}

	//todo why was this checked?
	for (set& s : sets)
	{
		assert(!(s.imageSamplers.empty() && s.inputAttachments.empty() && s.uniforms.empty()));
	}
}

VkPipelineLayout SbShaderLayout::reflect(vk::Device device, std::string vert, std::string frag,
	std::vector<VkPipelineShaderStageCreateInfo>& out)
{
	std::vector<uint32_t> vert_binary = loadSpirvBinary(vert);
	std::vector<uint32_t> frag_binary = loadSpirvBinary(frag);
	VkShaderModule vertModule = vks::helper::loadShader(vert.c_str(), device);//makeShaderModule(vert_binary, device);
	VkShaderModule fragModule = vks::helper::loadShader(frag.c_str(), device);//makeShaderModule(frag_binary, device);
	VkPipelineShaderStageCreateInfo vertCI = shaderStageCI(vertModule, VK_SHADER_STAGE_VERTEX_BIT, device);
	VkPipelineShaderStageCreateInfo fragCI = shaderStageCI(fragModule, VK_SHADER_STAGE_FRAGMENT_BIT, device);
	out.push_back(vertCI);
	out.push_back(fragCI);

	bool parseLayout = true;
	if (parseLayout)
	{
		parse(vert_binary, VK_SHADER_STAGE_VERTEX_BIT);
		parse(frag_binary, VK_SHADER_STAGE_FRAGMENT_BIT);

		DSL.resize(sets.size());
		bindings.resize(sets.size());

		for (size_t set = 0; set < sets.size(); set++)
		{
			auto& imageSamplers = sets[set].imageSamplers;
			auto& uniforms = sets[set].uniforms;
			auto& inputAttachments = sets[set].inputAttachments;

			//SbSetLayout layout;
			std::vector<VkDescriptorSetLayoutBinding>& currentSetBindings = bindings[set];

			for (auto& pair : uniforms)
				currentSetBindings.push_back(vkinit::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pair.second.stageFlags, pair.first));
			for (auto& pair : imageSamplers)
				currentSetBindings.push_back(vkinit::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pair.second.stageFlags, pair.first));
			for (auto& pair : inputAttachments)
				currentSetBindings.push_back(vkinit::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pair.second.stageFlags, pair.first));

			std::sort(currentSetBindings.begin(), currentSetBindings.end(),
				[](VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs)
				-> bool { return lhs.binding < rhs.binding; });

			VkDescriptorSetLayoutCreateInfo DSL_CI = vks::initializers::descriptorSetLayoutCreateInfo(
				currentSetBindings.data(), currentSetBindings.size());

			if (vkCreateDescriptorSetLayout(device, &DSL_CI, nullptr, &DSL[set]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
			//sbSetLayouts.push_back(createDSLayout(device, i));
		}
	}

	else 
	{
		DSL.resize(2);
		bindings.resize(2);

		//set 0
		bindings[0].push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
			VK_SHADER_STAGE_VERTEX_BIT, 0));

		VkDescriptorSetLayoutCreateInfo DSL_CI_0 = vks::initializers::descriptorSetLayoutCreateInfo(
			bindings[0].data(), bindings[0].size());

		if (vkCreateDescriptorSetLayout(device, &DSL_CI_0, nullptr, &DSL[0]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		//set 1		
		bindings[1].push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, 0));
		bindings[1].push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, 1));
		bindings[1].push_back(vkinit::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, 2));

		VkDescriptorSetLayoutCreateInfo DSL_CI_1 = vks::initializers::descriptorSetLayoutCreateInfo(
			bindings[1].data(), bindings[1].size());

		if (vkCreateDescriptorSetLayout(device, &DSL_CI_1, nullptr, &DSL[1]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		//------------------------------
		/*
		parse(vert_binary, VK_SHADER_STAGE_VERTEX_BIT);
		parse(frag_binary, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::vector<std::vector<VkDescriptorSetLayoutBinding>> parseBindings;
		std::vector<VkDescriptorSetLayout> parseDSL;
		parseDSL.resize(sets.size());
		parseBindings.resize(sets.size());

		for (size_t set = 0; set < sets.size(); set++)
		{
			auto& imageSamplers = sets[set].imageSamplers;
			auto& uniforms = sets[set].uniforms;
			auto& inputAttachments = sets[set].inputAttachments;

			//SbSetLayout layout;
			//std::vector<VkDescriptorSetLayoutBinding>& currentSetBindings = parseBindings[set];

			for (auto& pair : uniforms)
				parseBindings[set].push_back(vkinit::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pair.second.stageFlags, pair.first));
			for (auto& pair : imageSamplers)
				parseBindings[set].push_back(vkinit::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pair.second.stageFlags, pair.first));
			for (auto& pair : inputAttachments)
				parseBindings[set].push_back(vkinit::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pair.second.stageFlags, pair.first));

			std::sort(parseBindings[set].begin(), parseBindings[set].end(),
				[](VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs)
				-> bool { return lhs.binding < rhs.binding; });

			VkDescriptorSetLayoutCreateInfo DSL_CI = vks::initializers::descriptorSetLayoutCreateInfo(
				parseBindings[set].data(), parseBindings[set].size());

			if (vkCreateDescriptorSetLayout(device, &DSL_CI, nullptr, &DSL[set]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
			
			int cmp0 = std::memcmp((void*)&DSL_CI, (void*)&DSL_CI_0, sizeof(VkDescriptorSetLayoutCreateInfo));
			int cmp1 = std::memcmp((void*)&DSL_CI, (void*)&DSL_CI_1, sizeof(VkDescriptorSetLayoutCreateInfo));
						
			cmp0 = std::memcmp((void*)parseBindings[set].data(), (void*)bindings[0].data(), sizeof(VkDescriptorSetLayoutBinding) * parseBindings[set].size());
			cmp1 = std::memcmp((void*)parseBindings[set].data(), (void*)bindings[1].data(), sizeof(VkDescriptorSetLayoutBinding) * parseBindings[set].size());

			parseLayout = true;
		}
		*/

	}
	
	//std::vector<VkDescriptorSetLayout> setLayouts = std::vector<VkDescriptorSetLayout>(sbSetLayouts.size());
	//for (size_t i = 0; i < setLayouts.size(); i++)
	//	setLayouts[i] = sbSetLayouts[i].layout;

	VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(DSL.data(), DSL.size());
	//VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	//pipelineLayout = createPipelineLayout(device);
	return pipelineLayout;
}
