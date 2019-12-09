
#include "vulkan/vulkan.h"

#include <fstream>
#include <assert.h>
#include <iostream>

namespace vks
{
	namespace helper
	{
		VkShaderModule loadShader(const char *fileName, VkDevice device)
		{
			std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

			if (is.is_open())
			{
				size_t size = is.tellg();
				is.seekg(0, std::ios::beg);
				char* shaderCode = new char[size];
				is.read(shaderCode, size);
				is.close();

				assert(size > 0);

				VkShaderModule shaderModule;
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = size;
				moduleCreateInfo.pCode = (uint32_t*)shaderCode;

				vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);

				delete[] shaderCode;

				return shaderModule;
			}
			else
			{
				std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << std::endl;
				return VK_NULL_HANDLE;
			}
		}

		VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice device)
		{
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = loadShader(fileName.c_str(), device);
			shaderStage.pName = "main"; // todo : make param
			assert(shaderStage.module != VK_NULL_HANDLE);
			return shaderStage;
		}

	}
}