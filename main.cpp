

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <string>
#include <memory>

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "AnimationStuff.h"
#include "Model.h"

#include "Header.h"

#include "SbCamera.h"

#include "VulkanInitializers.hpp"

#include "VulkanHelperFunctions.hpp"

//#include "SbVulkanBase.h"



#include "SbCommandPool.h"

#include "SbLayout.h"
#include "SbDescriptorPool.h"
#include "SbPipelineLayout.h"

//#include "SbSwapchain.h"

#include "SbRenderpass.h"

#include "SbUniformBuffer.h"

namespace vkinit = vks::initializers;

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;


// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment)
{
	void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else 
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else 
	free(data);
#endif
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::mat4 boneTransforms[52];
};

struct ShadingUBO {
	/*alignas(16) glm::vec3 lightPosition;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec4 specularColor;
	alignas(16) glm::vec4 diffuseColor;
	alignas(16) glm::vec4 ambientColor;*/
	//alignas(256) 
	glm::vec4 specularColor;
	//alignas(256) 
	glm::vec4 diffuseColor;
	//alignas(256) 
	glm::vec4 ambientColor;
};

ShadingUBO* shadingUboData;


void indent(int indent) {
	for (size_t i = 0; i < indent; i++)
	{
		std::cout << "|  ";
	}
}

void PrintNode(const aiNode* node, const aiScene* scene, int nrindent) {
	indent(nrindent);
	std::cout << "name: " << node->mName.C_Str() << "\n";
	indent(nrindent);
	std::cout << "numchil: " << node->mNumChildren << "\n";
	indent(nrindent);
	std::cout << "nummesh: " << node->mNumMeshes << "\n";

	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		indent(nrindent);
		std::cout << "mesh index: " << node->mMeshes[i] << "\n";
		indent(nrindent);
		std::cout << "number of bones: " << scene->mMeshes[i]->mNumBones << "\n";
		for (size_t j = 0; j < scene->mMeshes[i]->mNumBones; j++)
		{
			indent(nrindent);
			std::cout << "mesh bones: " << scene->mMeshes[i]->mBones[j]->mName.C_Str() << "\n";
			
			indent(nrindent);
			std::cout << "vertexes affected: " << scene->mMeshes[i]->mBones[j]->mNumWeights << "\n";
		}
	}

	

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		PrintNode(node->mChildren[i],scene, nrindent + 1);
	}
}

class HelloTriangleApplication {
public:

	HelloTriangleApplication() 
		:cam(WIDTH,HEIGHT)
	{
		
	}

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

	~HelloTriangleApplication() {
		if (shadingUboData)
		{
			alignedFree(shadingUboData);
		}
		delete(mymodel);
	}

private:
	GLFWwindow* window;

	std::unique_ptr<SbVulkanBase> vulkanBase;
	std::unique_ptr<SbSwapchain> swapchain;	

	std::unique_ptr<SbRenderpass> renderPass;

	//struct SubpassSetup {
	//	std::unique_ptr<SbDescriptorSets> descriptorSets;
	//	VkPipeline Pipeline;
	//};
	//
	//SubpassSetup attachmentWriteSubpass;
	//SubpassSetup attachmentReadSubpass;

	//VkPipeline graphicsPipeline;

	//VkCommandPool commandPool;
	//std::unique_ptr<SbCommandPool> commandPool;

	enum
	{
		kAttachment_BACK,
		kAttachment_COLOR,
		kAttachment_DEPTH,
		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};
	enum
	{
		kSubpass_WRITE,
		kSubpass_READ,
		kSubpass_COUNT,
		kSubpass_MAX = kSubpass_COUNT - 1
	};

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	//custom model struct
	Model* mymodel = nullptr;

	//std::vector<VkBuffer> uniformBuffers;
	//std::vector<VkDeviceMemory> uniformBuffersMemory;

	std::unique_ptr<SbUniformBuffer<UniformBufferObject>> transformUniformBuffer;

	//std::vector<VkBuffer> shadingUniformBuffers;
	//std::vector<VkDeviceMemory> shadingUniformBuffersMemory;
	std::unique_ptr<SbUniformBuffer<ShadingUBO>> shadingUniformBuffer;

	std::vector<VkBuffer> transformationgUB;
	std::vector<VkDeviceMemory> transformationUBMemory;

	std::unique_ptr<SbDescriptorPool> descriptorPool;

	std::vector<VkCommandBuffer> commandBuffers;
	   
	bool framebufferResized = false;

	//size_t dynamicAlignment;
	//size_t dynamicBufferSize;

	SbCamera cam;


	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetKeyCallback(window, key_callback);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		//if (key == GLFW_KEY_E && action == GLFW_PRESS);
			//activate_airship();
		std::cout << "basic key callback" << std::endl;
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void initVulkan() {
		createInstance();
		//pickPhysicalDevice();
		//createLogicalDevice();
		createCommandPool();

		createSwapChain();
		createAttachmentResources();

		createRenderPass();
		createFramebuffers();

		createTextureImage();
		createTextureSampler();
		loadModel();
		createVertexBuffer();

		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSetLayout();
		//createGraphicsPipeline();
		createDoublePipeline();

		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(vulkanBase->logicalDevice->device);
	}

	void cleanupSwapChain() {

		for (size_t i = 0; i < swapchain->getSize(); i++)
		{
			//todo
			/*
			vkDestroyImageView(logicalDevice->device, attachmentSets[eSetIndex_Color].view[i], nullptr);
			vkDestroyImageView(logicalDevice->device, attachmentSets[eSetIndex_Depth].view[i], nullptr);
			vkDestroyImage(logicalDevice->device, attachmentSets[eSetIndex_Color].image[i], nullptr);
			vkDestroyImage(logicalDevice->device, attachmentSets[eSetIndex_Depth].image[i], nullptr); 
			vkFreeMemory(logicalDevice->device, attachmentSets[eSetIndex_Color].mem[i], nullptr);
			vkFreeMemory(logicalDevice->device, attachmentSets[eSetIndex_Depth].mem[i], nullptr);
			*/
		}

		/*vkDestroyImageView(device, depthAttachment.view, nullptr);
		vkDestroyImage(device, depthAttachment.image, nullptr);
		vkFreeMemory(device, depthAttachment.mem, nullptr);

		vkDestroyImageView(device, colorAttachment.view, nullptr);
		vkDestroyImage(device, colorAttachment.image, nullptr);
		vkFreeMemory(device, colorAttachment.mem, nullptr);*/

		for (auto framebuffer : swapchain->swapChainFramebuffers) {
			vkDestroyFramebuffer(vulkanBase->logicalDevice->device, framebuffer, nullptr);
		}

		vkFreeCommandBuffers(vulkanBase->logicalDevice->device, vulkanBase->commandPool->handle, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		//vkDestroyPipeline(vulkanBase->logicalDevice->device, attachmentWriteSubpass.Pipeline, nullptr);
		//vkDestroyPipeline(vulkanBase->logicalDevice->device, attachmentReadSubpass.Pipeline, nullptr);
		//vkDestroyPipelineLayout(vulkanBase->logicalDevice->device, attachmentWriteSubpass.descriptorSets.get()->pipelineLayout, nullptr);
		//vkDestroyPipelineLayout(vulkanBase->logicalDevice->device, attachmentReadSubpass.descriptorSets.get()->pipelineLayout, nullptr);
		vkDestroyRenderPass(vulkanBase->logicalDevice->device, renderPass->renderPass, nullptr);

		/*
		//TODO swapchain cleanup function
		for (auto imageView : swapchain->swapChainImageViews) {
			vkDestroyImageView(vulkanBase->logicalDevice->device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(vulkanBase->logicalDevice->device, swapchain->handle, nullptr);

		//TODO swapchain cleanup function
		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			vkDestroyBuffer(vulkanBase->logicalDevice->device, uniformBuffers[i], nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, uniformBuffersMemory[i], nullptr);
		}

		//TODO swapchain cleanup function
		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			vkDestroyBuffer(vulkanBase->logicalDevice->device, shadingUniformBuffers[i], nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, shadingUniformBuffersMemory[i], nullptr);
		}
		 */

		descriptorPool->destroy();
	}

	void cleanup() {
		cleanupSwapChain();

		vkDestroySampler(vulkanBase->logicalDevice->device, textureSampler, nullptr);
		vkDestroyImageView(vulkanBase->logicalDevice->device, textureImageView, nullptr);

		vkDestroyImage(vulkanBase->logicalDevice->device, textureImage, nullptr);
		vkFreeMemory(vulkanBase->logicalDevice->device, textureImageMemory, nullptr);

		//vkDestroyDescriptorSetLayout(vulkanBase->logicalDevice->device, attachmentWriteSubpass.descriptorSets.get()->DSLayout, nullptr);
		//vkDestroyDescriptorSetLayout(vulkanBase->logicalDevice->device, attachmentReadSubpass.descriptorSets.get()->DSLayout, nullptr);

		for (size_t i = 0; i < drawables.size(); i++)
		{
			vkDestroyBuffer(vulkanBase->logicalDevice->device, drawables[i].IndexBuffer, nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, drawables[i].IndexBufferMemory, nullptr);

			vkDestroyBuffer(vulkanBase->logicalDevice->device, drawables[i].VertexBuffer, nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, drawables[i].VertexBufferMemory, nullptr);
		}

		/*
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(vulkanBase->logicalDevice->device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(vulkanBase->logicalDevice->device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(vulkanBase->logicalDevice->device, inFlightFences[i], nullptr);
		}
		*/
		

		vkDestroyCommandPool(vulkanBase->logicalDevice->device, vulkanBase->commandPool->handle, nullptr);

		vkDestroyDevice(vulkanBase->logicalDevice->device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(vulkanBase->instance, vulkanBase->debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(vulkanBase->instance, vulkanBase->surface, nullptr);
		vkDestroyInstance(vulkanBase->instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vulkanBase->logicalDevice->device);

		cleanupSwapChain();

		createSwapChain();
		//createImageViews(); moved to swapchain creation
		createRenderPass();
		createDoublePipeline();
		createAttachmentResources();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorPool();
		//createDescriptorSets();
		createCommandBuffers();
	}

	void createInstance() {
		vulkanBase = std::make_unique<SbVulkanBase>();
		vulkanBase->createInstance(enableValidationLayers);
		vulkanBase->setupDebugMessenger(enableValidationLayers);//moved here from setupdebugmessenger
		vulkanBase->createSurface(window); //moved from createsurface

		vulkanBase->pickPhysicalDevice();
		vulkanBase->createLogicalDevice();
	}

	void createSwapChain() {
		swapchain = std::make_unique<SbSwapchain>(*vulkanBase);
		swapchain->createSwapChain(vulkanBase->surface, window, kAttachment_COUNT);
	}
	
	void createRenderPass() {
		//willems attachments demo
		
		renderPass = std::make_unique<SbRenderpass>(*vulkanBase, kSubpass_COUNT, kAttachment_COUNT, swapchain->getSize());

		renderPass->addAttachment(kAttachment_BACK, swapchain->getAttachmentDescription(kAttachment_BACK));
		renderPass->addAttachment(kAttachment_COLOR, swapchain->getAttachmentDescription(kAttachment_COLOR));
		renderPass->addAttachment(kAttachment_DEPTH, swapchain->getAttachmentDescription(kAttachment_DEPTH));

		renderPass->addColorAttachmentRef(kSubpass_WRITE, kAttachment_COLOR);
		renderPass->setDepthStencilAttachmentRef(kSubpass_WRITE, kAttachment_DEPTH);

		renderPass->addColorAttachmentRef(kSubpass_READ, kAttachment_BACK);
		renderPass->addInputAttachmentRef(kSubpass_READ, kAttachment_COLOR);
		renderPass->addInputAttachmentRef(kSubpass_READ, kAttachment_DEPTH);

		renderPass->addSyncMasks(kSubpass_WRITE, 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		renderPass->addSyncMasks(kSubpass_READ,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

		renderPass->addDependency(VK_SUBPASS_EXTERNAL, kSubpass_WRITE);
		renderPass->addDependency(kSubpass_WRITE, kSubpass_READ);
		renderPass->addDependency(kSubpass_READ, VK_SUBPASS_EXTERNAL);

		renderPass->createRenderpass(*swapchain);
	}

	void createDescriptorSetLayout() {
		//first create attachment write layout
		{
			//renderPass->subpasses[kSubpass_WRITE].pipelineLayout = std::make_unique<SbPipelineLayout>(vulkanBase->logicalDevice->device, swapchain->getSize());
			SbPipelineLayout & DS = renderPass->getPipelineLayout(kSubpass_WRITE);
			
			//create bindings
			DS.addBufferBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0), 
				*transformUniformBuffer);
			DS.addImageBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1), 
				{ 
					textureSampler, 
					&textureImageView, 
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					SbPipelineLayout::eSharingMode_Shared 
				});
			DS.addBufferBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2), 
				*shadingUniformBuffer);
			
			//with bindings created, create layout
			DS.createDSLayout();
			DS.createPipelineLayout();

			//allocate descriptors and update them with resources
			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors();
		}

		//now create attachment read layout
		{
			//renderPass->subpasses[kSubpass_READ].pipelineLayout = std::make_unique<SbPipelineLayout>(vulkanBase->logicalDevice->device, swapchain->getSize());
			SbPipelineLayout & DS = renderPass->getPipelineLayout(kSubpass_READ);

			DS.addImageBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
				{ 
					VK_NULL_HANDLE, 
					swapchain->getAttachmentViews(kAttachment_COLOR).data(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					SbPipelineLayout::eSharingMode_Separate 
				});
			DS.addImageBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
				{
					VK_NULL_HANDLE,
					swapchain->getAttachmentViews(kAttachment_COLOR).data(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
					SbPipelineLayout::eSharingMode_Separate 
				});
			//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			DS.createDSLayout();
			DS.createPipelineLayout();
			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors(); 
		}
	}	

	void createDoublePipeline() {
		auto & subpasswrite = renderPass->subpasses[kSubpass_WRITE];
		const auto & bind = Vertex::getBindingDescriptions();
		const auto & attr = Vertex::getAttributeDescriptions();
		subpasswrite.pipeline.subpassIndex(kSubpass_WRITE)
			.layout(subpasswrite.pipelineLayout.pipelineLayout)
			.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
			.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})
			.addShaderStage(vks::helper::loadShader("shaders/attachmentwrite.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, vulkanBase->logicalDevice->device))
			.addShaderStage(vks::helper::loadShader("shaders/attachmentwrite.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, vulkanBase->logicalDevice->device))
			.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);


		auto & subpassread = renderPass->subpasses[kSubpass_READ];
		subpassread.pipeline.subpassIndex(kSubpass_READ)
			.layout(subpassread.pipelineLayout.pipelineLayout)
			.addShaderStage(vks::helper::loadShader("shaders/attachmentread.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, vulkanBase->logicalDevice->device))
			.addShaderStage(vks::helper::loadShader("shaders/attachmentread.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, vulkanBase->logicalDevice->device))
			.cullMode(VK_CULL_MODE_NONE)
			.depthWriteEnable(VK_FALSE)
			.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);
		
	}
	
	void createFramebuffers() {
		swapchain->createFramebuffers(renderPass->renderPass);
	}
		
	void createCommandPool() {
		vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);
	}
	
	void createAttachmentResources() {
			//TODO USE SWAPCHAIN CREATE ATTACHMENT-----------------------------------------------------------
		swapchain->createAttachment(kAttachment_COLOR, swapchain->getAttachmentDescription(kAttachment_BACK).format,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		swapchain->createAttachment(kAttachment_DEPTH, vulkanBase->findDepthFormat(),
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

		//TODO new renderpass
		//createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.position);	// (World space) Positions		
		//createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.normal);		// (World space) Normals		
		//createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.albedo);			   
	}

	

	void createTextureImage() {

		//TODO move loading to model loading. get model with textures
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		vulkanBase->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(vulkanBase->logicalDevice->device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(vulkanBase->logicalDevice->device, stagingBufferMemory);

		stbi_image_free(pixels);

		vulkanBase->createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		vulkanBase->commandPool->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		vulkanBase->commandPool->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		vkDestroyBuffer(vulkanBase->logicalDevice->device, stagingBuffer, nullptr);
		vkFreeMemory(vulkanBase->logicalDevice->device, stagingBufferMemory, nullptr);

		vulkanBase->commandPool->generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);

		//createtextureimageview
		textureImageView = vks::helper::createImageView(vulkanBase->logicalDevice->device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	void createTextureSampler() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.mipLodBias = 0;

		if (vkCreateSampler(vulkanBase->logicalDevice->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
	
	Assimp::Importer modelImporter;
	const aiScene* modelScene;

	void loadModel() {

		modelScene = modelImporter.ReadFile("jump.fbx",
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_ValidateDataStructure);

		//PrintNode(modelScene->mRootNode, modelScene, 0);

		mymodel = new Model;

		//todo maybe use this with alignas(256)?
		//shadingUboData = new ShadingUBO[2];

		auto rootJoint = AnimationStuff::findRootJoint(modelScene->mRootNode, modelScene);
		auto totalNumberOfBones = mymodel->jointIndex.size();

		AnimationStuff::flattenJointHierarchy(rootJoint, *mymodel, 0, -1, modelScene);

		AnimationStuff::prepareInverseBindPose(mymodel->skeleton);

		if (modelScene->HasMeshes())
		{
			mymodel->meshes.resize(modelScene->mNumMeshes);
			for (unsigned int i = 0; i < modelScene->mNumMeshes; i++) {
				const aiMesh* currentMesh = modelScene->mMeshes[i];

				//read vertex data
				for (size_t j = 0; j < currentMesh->mNumVertices; j++)
				{
					glm::vec3 pos = {
							currentMesh->mVertices[j].x,
							currentMesh->mVertices[j].y,
							currentMesh->mVertices[j].z
					};


					glm::vec2 texCoords = { 0.0f, 0.0f };
					if (currentMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
					{
						texCoords.x = currentMesh->mTextureCoords[0][j].x;
						texCoords.y = currentMesh->mTextureCoords[0][j].y;
					}

					glm::vec3 color = { 1.0f, 1.0f, 1.0f };

					glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
					normal.x = currentMesh->mNormals[j].x;
					normal.y = currentMesh->mNormals[j].y;
					normal.z = currentMesh->mNormals[j].z;


					mymodel->meshes[i].vertexBuffer.emplace_back(
						pos,
						color,
						texCoords,
						normal
					);
				}

				//get vertex indices
				for (size_t j = 0; j < modelScene->mMeshes[i]->mNumFaces; j++)
				{
					for (size_t k = 0; k < modelScene->mMeshes[i]->mFaces[j].mNumIndices; k++)
					{
						mymodel->meshes[i].indexBuffer.push_back(modelScene->mMeshes[i]->mFaces[j].mIndices[k]);
					}
				}
				
				//add weights and bone indices to vertices
				for (size_t j = 0; j < currentMesh->mNumBones; j++)
				{
					size_t boneHierarchyIndex = mymodel->jointIndex[currentMesh->mBones[j]->mName.C_Str()];
					for (size_t k = 0; k < currentMesh->mBones[j]->mNumWeights; k++) {
						size_t meshVertexID = currentMesh->mBones[j]->mWeights[k].mVertexId;
						float Weight = currentMesh->mBones[j]->mWeights[k].mWeight;
						mymodel->meshes[i].vertexBuffer[meshVertexID].AddBoneData(boneHierarchyIndex, Weight);
					}
				}

				if (modelScene->HasMaterials())
				{
					auto material = modelScene->mMaterials[currentMesh->mMaterialIndex];

					aiColor4D specular;
					aiColor4D diffuse;
					aiColor4D ambient;
					float shine;

					aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
					aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
					aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
					aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shine);

					mymodel->meshes[i].material.diffuseColor = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
					mymodel->meshes[i].material.ambientColor = { ambient.r, ambient.g , ambient.b ,ambient.a };
					mymodel->meshes[i].material.specularColor = { specular.r, specular.g, specular.b, specular.a};
				}
			}
		}

	}
		
	//TODO make function take model argument?
	void createVertexBuffer() {

		for (size_t i = 0; i < mymodel->meshes.size(); i++)
		{
			VkDeviceSize bufferSize = sizeof(Vertex) * mymodel->meshes[i].vertexBuffer.size();
			VkDeviceSize indexBufferSize = sizeof(unsigned int) * mymodel->meshes[i].indexBuffer.size();

			//------
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			vulkanBase->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			VkBuffer stagingIndexBuffer;
			VkDeviceMemory stagingIndexBufferMemory;
			vulkanBase->createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);

			//------
			void* data;
			vkMapMemory(vulkanBase->logicalDevice->device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, mymodel->meshes[i].vertexBuffer.data(), (size_t)bufferSize);
			vkUnmapMemory(vulkanBase->logicalDevice->device, stagingBufferMemory);


			void* indexdata;
			vkMapMemory(vulkanBase->logicalDevice->device, stagingIndexBufferMemory, 0, indexBufferSize, 0, &indexdata);
			memcpy(indexdata, mymodel->meshes[i].indexBuffer.data(), (size_t)indexBufferSize);
			vkUnmapMemory(vulkanBase->logicalDevice->device, stagingIndexBufferMemory);
			//-----
			VkBuffer newVertexBuffer;
			VkDeviceMemory newVertexBufferMemory;
			vulkanBase->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newVertexBuffer, newVertexBufferMemory);

			VkBuffer newIndexBuffer;
			VkDeviceMemory newIndexBufferMemory;
			vulkanBase->createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newIndexBuffer, newIndexBufferMemory);
			
			//----
			drawables.emplace_back();
			drawables.back().id = drawables.size() - 1;
			drawables.back().VertexBuffer = newVertexBuffer;
			drawables.back().IndexBuffer = newIndexBuffer;
			drawables.back().VertexBufferMemory = newVertexBufferMemory;
			drawables.back().IndexBufferMemory = newIndexBufferMemory;
			drawables.back().IndexCount = mymodel->meshes[i].indexBuffer.size();
			drawables.back().AmbientColor = mymodel->meshes[i].material.ambientColor;
			drawables.back().DiffuseColor = mymodel->meshes[i].material.diffuseColor;
			drawables.back().SpecularColor = mymodel->meshes[i].material.specularColor;


			//vertexBuffers.push_back(newVertexBuffer);
			//vertexBufferMemory.push_back(newVertexBufferMemory);
			
			//indexBuffer.push_back(newIndexBuffer);
			//indexBufferMemory.push_back(newIndexBufferMemory);
			//-----

			vulkanBase->commandPool->copyBuffer(stagingBuffer, drawables.back().VertexBuffer, bufferSize);
			vulkanBase->commandPool->copyBuffer(stagingIndexBuffer, drawables.back().IndexBuffer, indexBufferSize);
			//----
			vkDestroyBuffer(vulkanBase->logicalDevice->device, stagingBuffer, nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, stagingBufferMemory, nullptr);

			vkDestroyBuffer(vulkanBase->logicalDevice->device, stagingIndexBuffer, nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, stagingIndexBufferMemory, nullptr);
		}
		
	}

	void createUniformBuffers() {
		transformUniformBuffer = std::make_unique<SbUniformBuffer<UniformBufferObject>>(*vulkanBase, swapchain->getSize()); 
				
		shadingUniformBuffer = std::make_unique<SbUniformBuffer<ShadingUBO>>(*vulkanBase, 1, drawables.size());

		for (size_t i = 0; i < drawables.size(); i++)
		{
			ShadingUBO data = { drawables[i].AmbientColor, drawables[i].DiffuseColor, drawables[i].SpecularColor };
			shadingUniformBuffer->writeBufferData(data, i);
		}
		shadingUniformBuffer->copyBufferDataToMemory(*vulkanBase);
	}

	void createDescriptorPool() {

		descriptorPool = std::make_unique<SbDescriptorPool>(vulkanBase->logicalDevice->device);

		std::vector<VkDescriptorPoolSize> poolSizes(4);
		poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapchain->getSize() * 2) }; //attachment write binding counts as uniform so there are 2 uniform bindings per frame, attachment and uniform struct
		poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapchain->getSize() * 2) };
		poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(swapchain->getSize() * 2) };
		poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(swapchain->getSize() * 2) };

		descriptorPool->createDescriptorPool(poolSizes, static_cast<uint32_t>(swapchain->getSize()) * 2);

		/*{
			std::vector<VkDescriptorPoolSize> poolSizes(4);
			poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapChainImages.size() * 2) }; //attachment write binding counts as uniform so there are 2 uniform bindings per frame, attachment and uniform struct
			poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapChainImages.size() * 2) };
			poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(swapChainImages.size() * 2) };
			poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(swapChainImages.size() * 2) };

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size()) * 2;

			if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}*/
	}

	void createCommandBuffers() {
		commandBuffers.resize(swapchain->swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = vulkanBase->commandPool->handle;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(vulkanBase->logicalDevice->device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass->renderPass;
			renderPassInfo.framebuffer = swapchain->swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapchain->swapchainCI.imageExtent;

			//std::array<VkClearValue, 2> clearValues = {};
			//clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			//clearValues[1].depthStencil = { 1.0f, 0 };

			VkClearValue clearValues[3];
			clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
			clearValues[1].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
			clearValues[2].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = 3;//static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport(
				(float)swapchain->swapchainCI.imageExtent.width, 
				(float)swapchain->swapchainCI.imageExtent.height, 0.0f, 1.0f);
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(swapchain->swapchainCI.imageExtent, 0, 0);
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_WRITE));

			for (size_t j = 0; j < drawables.size(); j++)
			{
				uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getPipelineLayout(kSubpass_WRITE).pipelineLayout,
					0, 1, &renderPass->getPipelineLayout(kSubpass_WRITE).allocatedDSs[i], 1, &offset);

				VkBuffer vert[] = { drawables[j].VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);

				//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
			}

			vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_READ));
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getPipelineLayout(kSubpass_READ).pipelineLayout, 0, 1, &renderPass->getPipelineLayout(kSubpass_READ).allocatedDSs[i], 0, NULL);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	//todo move this
	void createSyncObjects() {
		swapchain->createSyncObjects(MAX_FRAMES_IN_FLIGHT);
	}	

	void updateUniformBuffer(uint32_t currentImage) {

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float TicksPerSecond = modelScene->mAnimations[0]->mTicksPerSecond != 0 ?
			modelScene->mAnimations[0]->mTicksPerSecond : 25.0f;
		float TimeInTicks = time * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, modelScene->mAnimations[0]->mDuration);

		Skeleton& s = mymodel->skeleton;

		// the root has no parent
		s.localTransform[0] = AnimationStuff::makeAnimationMatrix(s.animationChannel[0], AnimationTime); //local animation transformation
		s.globalTransform[0] = s.localTransform[0];					//for root node local=global
		s.finalTransformation[0] = s.inverseBindPose[0] * s.globalTransform[0] * s.offsetMatrix[0];


		for (unsigned int i = 1; i < mymodel->jointIndex.size(); ++i)
		{
			const uint16_t parentJointIndex = s.hierarchy[i];
			s.localTransform[i] = AnimationStuff::makeAnimationMatrix(s.animationChannel[i], AnimationTime); //local animation transformation
			s.globalTransform[i] = s.globalTransform[parentJointIndex] * s.localTransform[i]; //animation transform in space of the parent
			s.finalTransformation[i] = s.inverseBindPose[1] * s.globalTransform[i] * s.offsetMatrix[i];
		}

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) 
																//* glm::translate(glm::mat4(1.0f), { 0.0f,-1.0f,0.0f })
			* glm::scale(glm::mat4(1.0f), { 0.01f,0.01f ,0.01f });//* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
		ubo.view = cam.getViewMatrix();
		ubo.proj = cam.getProjectionMatrix();
		memcpy(ubo.boneTransforms, mymodel->skeleton.finalTransformation.data(), sizeof(glm::mat4)*mymodel->skeleton.finalTransformation.size());


		transformUniformBuffer->writeBufferData(ubo);
		transformUniformBuffer->copyBufferDataToMemory(*vulkanBase);

		/*
		void* data;
		vkMapMemory(vulkanBase->logicalDevice->device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(vulkanBase->logicalDevice->device, uniformBuffersMemory[currentImage]);
		*/

	}

	void drawFrame() {
		uint32_t imageIndex = swapchain->acquireNextImage();		
		updateUniformBuffer(imageIndex);
		std::vector<VkCommandBuffer> cmds { commandBuffers[imageIndex] };
		std::vector<VkSemaphore> waitSem { swapchain->getImageAvailableSemaphore() };
		std::vector<VkSemaphore> signalSem { swapchain->getRenderFinishedSemaphore() };
		vulkanBase->submitCommandBuffers(cmds, waitSem, signalSem, swapchain->getInFlightFence());	
		swapchain->presentImage(imageIndex, signalSem);
		swapchain->updateFrameInFlightCounter();
	}
	
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

