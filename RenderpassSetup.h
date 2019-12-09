#pragma once

#include "boost/graph/adjacency_list.hpp"

#include "VulkanInitializers.hpp"

//#include "vulkan/vulkan_core.h"

class RenderpassSetup
{
public:


	class renderpass_properties {
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

	class renderpass_dependency {

	public:
		std::string name;
	};

	class input_dependency 
		: public renderpass_dependency {

	public:
		VkAttachmentReference inputReference = {};

		VkDescriptorSetLayoutBinding colorInputAttachmentLayoutBinding = {};
	};

	class output_dependency 
		: public renderpass_dependency {

	public:
		enum output_attachment_type {
			color,
			depth
		} output_type;
		VkAttachmentReference outputReference = {};
		//VkAttachmentReference colorReference = {};// { kAttachment_COLOR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		//VkAttachmentReference depthReference = {};// { kAttachment_DEPTH, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	};

	class subpass_dependency 
		: public renderpass_dependency {

	public:
		VkSubpassDependency dependency = {};
	};


	class renderpass_component {

	public:
		std::string name;
	};

	class attachment 
		: public renderpass_component
	{

	public:
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

	class subpass 
		: public renderpass_component
	{
	public:
		VkSubpassDescription description = {};


		std::vector<VkDescriptorSetLayoutBinding> bindings;
		
		//todo descriptorsetlayout
		//use these fields
		VkDescriptorSetLayout DS_Layout;
		std::vector <VkDescriptorSet> allocatedDSs;
		VkPipelineLayout Pipeline_Layout;
		VkPipeline Pipeline;

	};


	boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, renderpass_component, 
						  renderpass_dependency, renderpass_properties, boost::listS> adjList;

	RenderpassSetup();
	~RenderpassSetup();

	void addAttachment(const attachment & attachment) {
		boost::add_vertex(attachment, adjList);
	}

	void connect() {
		auto vertices = adjList.m_vertices;
		auto edges = adjList.m_edges;

		
		auto it = edges.begin();
		while (it != edges.end())
		{
			

			++it;
		}

	}


	void scratch() {
		
		//auto  = boost::add_vertex(attachment, adjList);
		
		attachment swapchainColorImage;

		//swapchainColorImage.description.format = swapChainImageFormat;
		swapchainColorImage.description.samples = VK_SAMPLE_COUNT_1_BIT;
		swapchainColorImage.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		swapchainColorImage.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		swapchainColorImage.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		swapchainColorImage.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		swapchainColorImage.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		swapchainColorImage.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		attachment inputColor;

		//inputColor.description.format = swapChainImageFormat;
		inputColor.description.samples = VK_SAMPLE_COUNT_1_BIT;
		inputColor.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		inputColor.description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		inputColor.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		inputColor.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		inputColor.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		inputColor.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachment inputDepth;

		//inputDepth.description.format = findDepthFormat();
		inputDepth.description.samples = VK_SAMPLE_COUNT_1_BIT;
		inputDepth.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		inputDepth.description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		inputDepth.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		inputDepth.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		inputDepth.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		inputDepth.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		


		output_dependency colorAttachmentOutputDependecy;
		colorAttachmentOutputDependecy.outputReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		output_dependency depthAttachmentOutputDependecy;
		depthAttachmentOutputDependecy.outputReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		input_dependency colorAttachmentInputDependecy;
		colorAttachmentInputDependecy.inputReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
		input_dependency depthAttachmentInputDependecy;
		depthAttachmentInputDependecy.inputReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 


		subpass attachmentWriteSubpass;
		attachmentWriteSubpass.name = "attachment write subpass";

		attachmentWriteSubpass.description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		attachmentWriteSubpass.description.colorAttachmentCount = 1;
		//attachmentWriteSubpass.description.pColorAttachments = &colorReference;
		//attachmentWriteSubpass.description.pDepthStencilAttachment = &depthReference;

		attachmentWriteSubpass.bindings.resize(3);

		attachmentWriteSubpass.bindings[0] = vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
		attachmentWriteSubpass.bindings[1] = vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		attachmentWriteSubpass.bindings[2] = vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2);


		subpass attachmentReadSubpass;
		attachmentReadSubpass.name = "attachment read subpass";

		attachmentReadSubpass.description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		attachmentReadSubpass.description.colorAttachmentCount = 1;
		//attachmentReadSubpass.description.pColorAttachments = &colorReferenceSwapchain;
		attachmentReadSubpass.description.inputAttachmentCount = 2;
		//attachmentReadSubpass.description.pInputAttachments = inputReferences;
		// Use the attachments filled in the first pass as input attachments


		subpass_dependency begin;
		//begin.dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		//begin.dependency.dstSubpass = 0;
		begin.dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		begin.dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		begin.dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		begin.dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		begin.dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpass_dependency writeToRead;
		//writeToRead.dependency.srcSubpass = 0;
		//writeToRead.dependency.dstSubpass = 1;
		writeToRead.dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		writeToRead.dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		writeToRead.dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		writeToRead.dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		writeToRead.dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpass_dependency end;
		//end.dependency.srcSubpass = 0;
		//end.dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		end.dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		end.dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		end.dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		end.dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		end.dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


		renderpass_properties renderPassProp;
		renderPassProp.renderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		//renderPassProp.renderpassCI.attachmentCount = static_cast<uint32_t>(attachments.size());
		//renderPassProp.renderpassCI.pAttachments = attachments.data();
		//renderPassProp.renderpassCI.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
		//renderPassProp.renderpassCI.pSubpasses = subpassDescriptions.data();
		//renderPassProp.renderpassCI.dependencyCount = static_cast<uint32_t>(dependencies.size());
		//renderPassProp.renderpassCI.pDependencies = dependencies.data();


	};

};

