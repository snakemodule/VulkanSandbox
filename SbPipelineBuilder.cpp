#include "SbPipelineBuilder.h"
#include "VulkanInitializers.hpp"
#include "VulkanHelperFunctions.h"


//#include "spirv-cross/spirv_common.hpp"

#include "spirvloader.h"

#include <iostream>

#include "TestRenderpass.h"
#include "DescriptorSetLayoutCache.h"
#include "DescriptorAllocator.h"

SbPipelineBuilder& SbPipelineBuilder::setRasterizationState(vk::PolygonMode polygonMode)
{
	_rasterizer = vks::initializers::pipelineRasterizationStateCreateInfo(polygonMode);
	return *this;
}

SbPipelineBuilder& SbPipelineBuilder::setInputAssembly(vk::PrimitiveTopology topology)
{
	_inputAssembly = vks::initializers::pipelineInputAssemblyStateCreateInfo(topology);
	return *this;
}

SbPipelineBuilder& SbPipelineBuilder::setViewport(int width, int height)
{
	_viewport.x = 0.0f;
	_viewport.y = 0.0f;
	_viewport.width = (float)width;
	_viewport.height = (float)height;
	_viewport.minDepth = 0.0f;
	_viewport.maxDepth = 1.0f;

	_scissor.offset = { 0, 0 };
	_scissor.extent = vk::Extent2D(width, height);
	return *this;
}

SbPipelineBuilder& SbPipelineBuilder::setAttachmentColorBlend(vk::PipelineColorBlendAttachmentState blend, unsigned index)
{
	_colorBlendAttachment[index] = blend;
	return *this;
}


SbPipelineBuilder& SbPipelineBuilder::shaderReflection(vk::Device device, const char* vertPath, const char* fragPath,
	DescriptorSetLayoutCache& DSL_cache, SbSwapchain& swapchain, SbRenderpass& rp, uint32_t subpassID)
{
	namespace vkinit = vks::initializers;

	//const char* vertName = "shaders/gbuf.vert.spv";
	//const char* fragName = "shaders/gbuf.frag.spv";


	std::vector<uint32_t> vert_binary = loadSpirvBinary(vertPath);
	spirv_cross::CompilerGLSL vert(std::move(vert_binary));

	std::vector<uint32_t> frag_binary = loadSpirvBinary(fragPath);
	spirv_cross::CompilerGLSL frag(std::move(frag_binary));


	//_vertexInputInfo
	reflectVertexInput(vert);

	getDescriptorSetResources(vert, vk::ShaderStageFlagBits::eVertex);
	getDescriptorSetResources(frag, vk::ShaderStageFlagBits::eFragment);

	reflectFragmentOutputs(frag);

	

	DSL.resize(sets.size());
	bindings.resize(sets.size());

	for (size_t set = 0; set < sets.size(); set++)
	{
		auto& imageSamplers = sets[set].imageSamplers;
		auto& uniforms = sets[set].uniforms;
		auto& inputAttachments = sets[set].inputAttachments;

		std::vector<vk::DescriptorSetLayoutBinding>& currentSetBindings = bindings[set];

		for (auto& pair : uniforms)
			currentSetBindings.push_back(vkinit::descriptorSetLayoutBinding(
				vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlags(pair.second.stageFlags), pair.first));
		for (auto& pair : imageSamplers)
			currentSetBindings.push_back(vkinit::descriptorSetLayoutBinding(
				vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlags(pair.second.stageFlags), pair.first));
		for (auto& pair : inputAttachments)
			currentSetBindings.push_back(vkinit::descriptorSetLayoutBinding(
				vk::DescriptorType::eInputAttachment, vk::ShaderStageFlags(pair.second.stageFlags), pair.first));

		std::sort(currentSetBindings.begin(), currentSetBindings.end(),
			[](VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs)
			-> bool { return lhs.binding < rhs.binding; });

		vk::DescriptorSetLayoutCreateInfo DSL_CI = vks::initializers::descriptorSetLayoutCreateInfo(
			currentSetBindings.data(), currentSetBindings.size());
				
		DSL[set] = DSL_cache.create_descriptor_layout(&DSL_CI);
	}
	
	vk::PipelineLayoutCreateInfo pipeline_layout_info = vks::initializers::pipelineLayoutCreateInfo(DSL.data(), DSL.size());
	if (push_constant.size != 0)
	{
		pipeline_layout_info.pPushConstantRanges = &push_constant;
		pipeline_layout_info.pushConstantRangeCount = 1;
	}
	_pipelineLayout = device.createPipelineLayout(pipeline_layout_info);

	auto shaderStageCI = [](vk::ShaderModule shaderModule, vk::ShaderStageFlagBits stage, vk::Device device)
		-> vk::PipelineShaderStageCreateInfo
	{
		vk::PipelineShaderStageCreateInfo shaderStage;
		shaderStage.stage = stage;
		shaderStage.module = shaderModule;
		shaderStage.pName = "main";
		return shaderStage;
	};

	vk::ShaderModule vertModule = vks::helper::loadShader(vertPath, device);
	vk::ShaderModule fragModule = vks::helper::loadShader(fragPath, device);
	vk::PipelineShaderStageCreateInfo vertCI = shaderStageCI(vertModule, vk::ShaderStageFlagBits::eVertex, device);
	vk::PipelineShaderStageCreateInfo fragCI = shaderStageCI(fragModule, vk::ShaderStageFlagBits::eFragment, device);
	_shaderStages = { vertCI, fragCI };

	this->subpassID = subpassID;

	//uint32_t expectedInputAttachmentCount = rp.subpasses[subpassID].inputAttachments.size();
	//std::unordered_map<uint64_t,std::vector<vk::DescriptorImageInfo>> input_attachments_descriptor_info(expectedInputAttachmentCount);

	auto& attachments = rp.subpasses[subpassID].inputAttachments;
	for (uint64_t set = 0; set < sets.size(); set++)
	{
		for (auto it = sets[set].inputAttachments.begin(); it != sets[set].inputAttachments.end(); it++)
		{			
			uint32_t binding = it->first;
			SubpassInput subpass_input = it->second;

			uint64_t set_binding_combo = (set << 32) | (uint64_t)binding;
			uint32_t input_attachment_index = subpass_input.input_attachment_index;
						
			std::vector<vk::DescriptorImageInfo>& attachment_instance_infos = input_attachments_descriptor_info.emplace(
				set_binding_combo, std::vector<vk::DescriptorImageInfo>{}).first->second;
		
			auto& views = swapchain.getAttachmentViews(rp.subpasses[subpassID].inputAttachments[input_attachment_index].attachment);
			for (size_t j = 0; j < views.size(); j++)
			{
				vk::DescriptorImageInfo attachment_image_info;
				attachment_image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				attachment_image_info.imageView = views[j];
				attachment_instance_infos.push_back(attachment_image_info);
			}
		}
	}

	auto& colorAttachments = rp.subpasses[subpassID].colorAttachments;
	_colorBlendAttachment = std::vector<vk::PipelineColorBlendAttachmentState>(colorAttachments.size(), vks::initializers::pipelineColorBlendAttachmentState());

	return *this;
}



vk::Pipeline SbPipelineBuilder::build_pipeline(vk::Device device, vk::RenderPass pass, uint32_t subpass)
{
	//make viewport state from our stored viewport and scissor.
		//at the moment we won't support multiple viewports or scissors
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.pNext = nullptr;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &_viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &_scissor;

	//setup dummy color blending. We aren't using transparent objects yet
	//the blending is just "no blend", but we do write to the color attachment
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.pNext = nullptr;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = _colorBlendAttachment.size();
	colorBlending.pAttachments = _colorBlendAttachment.data();

	//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.stageCount = _shaderStages.size();
	pipelineInfo.pStages = _shaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &_rasterizer;
	pipelineInfo.pMultisampleState = &_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = subpassID;
	pipelineInfo.basePipelineHandle = vk::Pipeline();

	


	//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	vk::Pipeline pipeline;
	vk::Result res = device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipeline);
	if (res != vk::Result::eSuccess) {
		std::cout << "failed to create pipeline\n";
		return vk::Pipeline(); // failed to create graphics pipeline
	}
	else
	{
		return pipeline;
	}
}

//void SbPipelineBuilder::reflectDescriptorSets(spirv_cross::CompilerGLSL& vert, spirv_cross::CompilerGLSL& frag)
//{
//	
//}

void SbPipelineBuilder::reflectVertexInput(spirv_cross::CompilerGLSL& glsl)
{
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = 0;
	vertexBindingDescription.inputRate = vk::VertexInputRate::eVertex;

	for (auto& resource : resources.stage_inputs) {
		const auto& type = glsl.get_type(resource.type_id);
		if (type.basetype == spirv_cross::SPIRType::BaseType::Float)
		{
			vk::VertexInputAttributeDescription& newInput = vertexAttributeDescriptions.emplace_back();
			newInput.binding = 0;
			newInput.location = glsl.get_decoration(resource.id, spv::DecorationLocation);
			switch (type.vecsize)
			{
			case 1:
				newInput.format = vk::Format::eR32Sfloat;
				vertexBindingDescription.stride += 1 * sizeof(float);
				break;
			case 2:
				newInput.format = vk::Format::eR32G32Sfloat;
				vertexBindingDescription.stride += 2 * sizeof(float);
				break;
			case 3:
				newInput.format = vk::Format::eR32G32B32Sfloat;
				vertexBindingDescription.stride += 3 * sizeof(float);
				break;
			case 4:
				newInput.format = vk::Format::eR32G32B32A32Sfloat;
				vertexBindingDescription.stride += 4 * sizeof(float);
				break;
			default:
				throw std::runtime_error("unsupported shader input!");
				break;
			}
		}
		else {
			throw std::runtime_error("unsupported shader input!");
		}
	}

	std::sort(vertexAttributeDescriptions.begin(), vertexAttributeDescriptions.end(),
		[](vk::VertexInputAttributeDescription lhs, vk::VertexInputAttributeDescription rhs)
		-> bool { return lhs.location < rhs.location; });

	int offsetCounter = 0;
	for (size_t i = 0; i < vertexAttributeDescriptions.size(); i++)
	{
		vertexAttributeDescriptions[i].offset = offsetCounter;
		switch (vertexAttributeDescriptions[i].format)
		{
		case vk::Format::eR32Sfloat:
			offsetCounter += 1 * sizeof(float);
			break;
		case vk::Format::eR32G32Sfloat:
			offsetCounter += 2 * sizeof(float);
			break;
		case vk::Format::eR32G32B32Sfloat:
			offsetCounter += 3 * sizeof(float);
			break;
		case vk::Format::eR32G32B32A32Sfloat:
			offsetCounter += 4 * sizeof(float);
			break;
		default:
			throw std::runtime_error("unsupported shader input!");
			break;
		}
	}

	if (vertexAttributeDescriptions.size())
	{
		_vertexInputInfo.vertexBindingDescriptionCount = 1;
		_vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
		_vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
		_vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
	}
	else {
		_vertexInputInfo.vertexBindingDescriptionCount = 0;
		_vertexInputInfo.pVertexBindingDescriptions = nullptr;
		_vertexInputInfo.vertexAttributeDescriptionCount = 0;
		_vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	}


}

void SbPipelineBuilder::getDescriptorSetResources(spirv_cross::CompilerGLSL& glsl, vk::ShaderStageFlagBits shaderStage)
{
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
			it->second.stageFlags |= (uint64_t)shaderStage;
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
			it->second.stageFlags |= (uint64_t)shaderStage;
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
			it->second.stageFlags |= (uint64_t)shaderStage;
		else
			setUniforms[binding] = { resource.name, (uint64_t)shaderStage, (unsigned)size };

		printf("Uniform buffer %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
	}

	for (auto& resource : resources.push_constant_buffers) {
		const spirv_cross::SPIRType& type = glsl.get_type(resource.base_type_id);
		size_t size = glsl.get_declared_struct_size(type);
				
		//this push constant range starts at the beginning
		push_constant.offset = 0;
		//this push constant range takes up the size of a MeshPushConstants struct
		push_constant.size = size;
		//this push constant range is accessible only in the vertex shader
		push_constant.stageFlags |= shaderStage;
	}

	//todo why was this checked?
	for (set& s : sets)
	{
		assert(!(s.imageSamplers.empty() && s.inputAttachments.empty() && s.uniforms.empty() && push_constant.size == 0));
	}
}

void SbPipelineBuilder::reflectFragmentOutputs(spirv_cross::CompilerGLSL& frag)
{
	spirv_cross::ShaderResources resources = frag.get_shader_resources();

	if (resources.stage_outputs.empty())
		return;


	_colorBlendAttachment = {};

	for (auto& resource : resources.stage_outputs)
	{
		unsigned location = frag.get_decoration(resource.id, spv::DecorationLocation);
		const auto& type = frag.get_type(resource.type_id);

		vk::PipelineColorBlendAttachmentState newBlendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState();

		switch (type.vecsize)
		{
		case 4:
			newBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
			break;
		case 3:
			newBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
			break;
		case 2:
			newBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;
			break;
		case 1:
			newBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR;
			break;
		default:
			break;
		}
		_colorBlendAttachment.push_back(newBlendAttachmentState);
	}

}
