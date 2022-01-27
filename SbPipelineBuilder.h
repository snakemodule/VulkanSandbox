#pragma once
#include "vulkan/vulkan.hpp"

#include "VulkanInitializers.hpp"

#include "spirv-cross/spirv_cross.hpp"
#include "spirv-cross/spirv_reflect.hpp"

#include "SbSwapchain.h"
#include "SbRenderpass.h"
#include "DescriptorSetLayoutCache.h"

#include <map>
#include <unordered_map>

class SbPipelineBuilder
{

public:

	std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages;
	vk::PipelineVertexInputStateCreateInfo _vertexInputInfo; //reflectVertexInput
	vk::PipelineInputAssemblyStateCreateInfo _inputAssembly = vks::initializers::pipelineInputAssemblyStateCreateInfo(vk::PrimitiveTopology::eTriangleList);
	vk::Viewport _viewport;
	vk::Rect2D _scissor;
	vk::PipelineRasterizationStateCreateInfo _rasterizer = vks::initializers::pipelineRasterizationStateCreateInfo(vk::PolygonMode::eFill);
	std::vector<vk::PipelineColorBlendAttachmentState> _colorBlendAttachment = { vks::initializers::pipelineColorBlendAttachmentState() };
	vk::PipelineMultisampleStateCreateInfo _multisampling = vks::initializers::pipelineMultisampleStateCreateInfo();
	vk::PipelineLayout _pipelineLayout; //reflect
	
	uint32_t subpassID;

	std::unordered_map<uint64_t, std::vector<vk::DescriptorImageInfo>> input_attachments_descriptor_info;

	std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings;
	std::vector<vk::DescriptorSetLayout> DSL;
		
	SbPipelineBuilder& setRasterizationState(vk::PolygonMode polygonMode);
	SbPipelineBuilder& setInputAssembly(vk::PrimitiveTopology topology);
	SbPipelineBuilder& setViewport(int width, int height);

	SbPipelineBuilder& shaderReflection(vk::Device device, const char* vertPath, const char* fragPath,
		DescriptorSetLayoutCache& DSL_cache, SbSwapchain& swapchain, SbRenderpass& rp, uint32_t subpassID);

	vk::Pipeline build_pipeline(vk::Device device, vk::RenderPass pass, uint32_t subpass);

private:
	std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
	vk::VertexInputBindingDescription vertexBindingDescription;


	void reflectDescriptorSets(spirv_cross::CompilerGLSL& vert, spirv_cross::CompilerGLSL& frag);
	void reflectVertexInput(spirv_cross::CompilerGLSL& glsl);

	struct SubpassInput
	{
		std::string name;
		uint64_t stageFlags = 0;
		unsigned input_attachment_index = 0;
	};

	struct UniformBuffer
	{
		std::string name;
		uint64_t stageFlags = 0;
		unsigned size = 0;
	};

	struct SampledImage
	{
		std::string name;
		uint64_t stageFlags = 0;
	};

	struct set {
		std::map<unsigned, UniformBuffer> uniforms;
		std::map<unsigned, SubpassInput> inputAttachments;
		std::map<unsigned, SampledImage> imageSamplers;
	};
	std::vector<set> sets;

	

	void getDescriptorSetResources(spirv_cross::CompilerGLSL& glsl, VkShaderStageFlagBits shaderStage);

	void reflectFragmentOutputs(spirv_cross::CompilerGLSL& frag);

};

