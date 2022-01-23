#pragma once


#include "vulkan/vulkan.hpp"
#include <map>


class SbShaderLayout
{


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

public:

	struct SbSetLayout {
		std::vector<VkDescriptorSetLayoutBinding> bindingInfo;
		VkDescriptorSetLayout layout;
	};
	//std::vector<SbSetLayout> sbSetLayouts;

	std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
	std::vector<VkDescriptorSetLayout> DSL;

	VkPipelineLayout pipelineLayout;


	SbSetLayout createDSLayout(vk::Device device, int set);
	//VkPipelineLayout createPipelineLayout(vk::Device device);

	void parse(std::vector<uint32_t>& spirv_binary, VkShaderStageFlagBits shaderStage);
	VkPipelineLayout reflect(vk::Device device, std::string vert, std::string frag, 
		std::vector<VkPipelineShaderStageCreateInfo>& out);





	//std::vector<VkDescriptorSetLayoutBinding> bindings;
	//std::vector<VkDescriptorSetLayout> setLayouts;
};

