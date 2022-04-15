#pragma once


#include "vulkan/vulkan.hpp"
#include <map>


class SbShaderLayout
{
public:

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

	struct StorageBuffer {
		std::string name;
		uint64_t stageFlags = 0;
	};

	struct SampledImage
	{
		std::string name;
		uint64_t stageFlags = 0;
	};

	struct set {
		std::map<unsigned, UniformBuffer> uniforms;
		std::map<unsigned, StorageBuffer> storageBuffers;
		std::map<unsigned, SubpassInput> inputAttachments;
		std::map<unsigned, SampledImage> imageSamplers;
	};
	std::vector<set> sets;
		
	struct {
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindingInfo;
		std::vector<VkDescriptorSetLayout> setLayouts;
		//std::array<VkPipelineShaderStageCreateInfo, 2> shaderInfo;
		std::vector<VkPipelineShaderStageCreateInfo> shaderInfo;
		VkPipelineLayout pipelineLayout;
	} results;	

	void createDSLayout(vk::Device device, int set);
	void createPipelineLayout(vk::Device device);

	void parse(std::vector<uint32_t>& spirv_binary, VkShaderStageFlagBits shaderStage);
	void reflect(vk::Device device, std::string vert, std::string frag);
	void reflect_nofrag(vk::Device device, std::string vert);
	void reflect_compute(vk::Device device, std::string compute);

	void addBuffer() {

	}




	//std::vector<VkDescriptorSetLayoutBinding> bindings;
	//std::vector<VkDescriptorSetLayout> setLayouts;
};

