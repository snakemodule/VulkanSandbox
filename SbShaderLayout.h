#pragma once


#include "vulkan/vulkan.hpp"
#include <map>


class SbShaderLayout
{
public:

	struct SubpassInput
	{
		uint64_t stageFlags = 0;
		unsigned input_attachment_index = 0;
	};

	struct UniformBuffer
	{
		uint64_t stageFlags = 0;
		unsigned size = 0;
	};

	struct SampledImage
	{
		uint64_t stageFlags = 0;
	};

	struct set {
		std::map<unsigned, UniformBuffer> uniforms;
		std::map<unsigned, SubpassInput> inputAttachments;
		std::map<unsigned, SampledImage> imageSamplers;
	};

	struct SbSetLayout {
		std::vector<VkDescriptorSetLayoutBinding> bindingInfo;
		VkDescriptorSetLayout layout;
	};

	std::vector<set> sets;
	VkPipelineLayout pipelineLayout;
	std::vector<SbSetLayout> sbSetLayouts;


	SbSetLayout createDSLayout(vk::Device device, int set);
	VkPipelineLayout createPipelineLayout(vk::Device device);

	void parse(std::vector<uint32_t>& spirv_binary, VkShaderStageFlagBits shaderStage);
	VkPipelineLayout reflect(vk::Device device, std::string vert, std::string frag, 
		std::vector<VkPipelineShaderStageCreateInfo>& out);





	//std::vector<VkDescriptorSetLayoutBinding> bindings;
	//std::vector<VkDescriptorSetLayout> setLayouts;
};

