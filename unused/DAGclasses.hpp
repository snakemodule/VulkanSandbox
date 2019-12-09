#pragma once

#include "boost/graph/adjacency_list.hpp"

#include "VulkanInitializers.hpp"



class SbRenderpassProperties {
public:
	VkRenderPassCreateInfo renderpassCI = {};

	VkDescriptorPoolCreateInfo poolInfo = {};

	struct common_pipeline_properties {
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationStateCI = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS); //VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportStateCI = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleStateCI = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo();
	};

};



class SbInputDependency
{
public:
	SbAttachment srcAttachment;
	SbSubpass dstSubpass;


	VkAttachmentReference inputReference = {};

	VkDescriptorSetLayoutBinding colorInputAttachmentLayoutBinding = {};
};

class SbOutputDependency
{
public:
	SbAttachment dstAttachment;
	SbSubpass srcSubpass;

	enum output_attachment_type {
		color,
		depth
	} output_type;
	VkAttachmentReference outputReference = {};
	//VkAttachmentReference colorReference = {};// { kAttachment_COLOR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	//VkAttachmentReference depthReference = {};// { kAttachment_DEPTH, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
};

class SbSubpassDependency
{
public:
	SbSubpass srcSubpass;
	SbSubpass dstSubpass;

	VkSubpassDependency dependency = {};
};

class SbAttachment
{

public:
	std::vector<SbOutputDependency> fromSubpass; //how many?
	std::vector<SbInputDependency> toSubpass;


	VkAttachmentDescription description = {}; //
	VkAttachmentReference colorReference = {};

	VkFormat attachmentFormat;
	VkImage attachmentImage;
	VkDeviceMemory mem;
	VkImageView view;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

	//createimage
	VkExtent2D extent;
	uint32_t miplevels = 1;
	VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
	uint32_t imageUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	VkMemoryPropertyFlags memProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageLayout untransitionedLayout = VK_IMAGE_LAYOUT_UNDEFINED; //maybe this is not needed
	VkImageLayout transitionedLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
};

class SbSubpass
{
public:
	std::vector<SbSubpassDependency> precedingSubpasses;
	std::vector<SbSubpassDependency> subsequentSubpasses;

	std::vector<SbInputDependency> attachmentInputs;
	std::vector<SbOutputDependency> attachmentOutputs;
	SbOutputDependency depthStencilAttachment;




	VkSubpassDescription description = {};


	std::vector<VkDescriptorSetLayoutBinding> bindings;

	//todo descriptorsetlayout
	//use these fields
	VkDescriptorSetLayout DS_Layout;
	std::vector <VkDescriptorSet> DSs;
	VkPipelineLayout Pipeline_Layout;
	VkPipeline Pipeline;


	VkSubpassDescription buildSubpassDescription() {
		auto input = getInputAttachmentReferences();
		auto output = getOutputAttachmentReferences();
		VkSubpassDescription desc = {};
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.inputAttachmentCount = static_cast<uint32_t>(input.size());
		desc.pInputAttachments = input.data();
		desc.colorAttachmentCount = static_cast<uint32_t>(output.size());;
		desc.pColorAttachments = output.data();
		desc.pDepthStencilAttachment = &depthStencilAttachment.outputReference;
		//todo 
		//resolve attachments
		//preserve attch
		

		return desc;
	}

	


	std::vector<VkAttachmentReference> getInputAttachmentReferences() {
		auto it = attachmentInputs.begin();
		std::vector<VkAttachmentReference> inputReferences;
		while (it != attachmentInputs.end())
		{
			inputReferences.push_back(it->inputReference);
			++it;
		}
		return inputReferences;
	}

	std::vector<VkAttachmentReference> getOutputAttachmentReferences() {
		auto it = attachmentOutputs.begin();
		std::vector<VkAttachmentReference> outputReferences;
		while (it != attachmentOutputs.end())
		{
			outputReferences.push_back(it->outputReference);
			++it;
		}
		return outputReferences;
	}

};




