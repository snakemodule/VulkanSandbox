

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

#include <cmath>

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "AnimationStuff.h"

#include "SkeletalAnimationComponent.h"
#include "Model.h"

#include "Header.h"

#include "SbCamera.h"

#include "VulkanInitializers.hpp"

#include "VulkanHelperFunctions.hpp"

//#include "SbVulkanBase.h"

#include "AnimationKeys.h"
#include "UncompressedAnimationKeys.h"
#include "UncompressedAnimation.h"

#include "SbCommandPool.h"

#include "SbLayout.h"
#include "SbDescriptorPool.h"
#include "SbPipelineLayout.h"

//#include "SbSwapchain.h"

#include "SbRenderpass.h"
#include "MyRenderPass.h"

#include "SbUniformBuffer.h"
#include "SbImage.h"
#include "SbTextureImage.h"

#include "vulkan/vulkan.hpp"


#include "spirv-cross/spirv_reflect.hpp"
#include "spirvloader.h"


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


//struct DefaultSkeletonTransformUBO {
//	alignas(16) glm::mat4 boneTransforms[52]; 
//	//this is only the size of the skeleton, the animation system does not output whole skeletons contigously
//};


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
		kAttachment_POSITION,
		kAttachment_NORMAL,
		kAttachment_ALBEDO,
		kAttachment_DEPTH,
		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};

	/*
	enum
	{
		kAttachment_BACK,
		kAttachment_COLOR,
		kAttachment_DEPTH,
		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_COUNT - 1
	};
	*/
	enum
	{
		kSubpass_GBUF,
		kSubpass_COMPOSE,
		kSubpass_TRANSPARENT,
		kSubpass_COUNT,
		kSubpass_MAX = kSubpass_COUNT - 1
	};

	//uint32_t mipLevels;
	//VkImage textureImage;
	//VkDeviceMemory textureImageMemory;
	//VkImageView textureImageView;
	std::unique_ptr<SbTextureImage> texture;

	vk::Sampler textureSampler;

	//custom model struct
	Model* mymodel = nullptr;

	//std::vector<VkBuffer> uniformBuffers;
	//std::vector<VkDeviceMemory> uniformBuffersMemory;

	std::unique_ptr<SbUniformBuffer<UniformBufferObject>> transformUniformBuffer;

	//std::vector<VkBuffer> shadingUniformBuffers;
	//std::vector<VkDeviceMemory> shadingUniformBuffersMemory;
	std::unique_ptr<SbUniformBuffer<ShadingUBO>> shadingUniformBuffer;
	
	std::unique_ptr <SbUniformBuffer<glm::mat4>> skeletonUniformBuffer;

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
		createPipelines();

		createCommandBuffers();
		createSyncObjects();
	}

	std::chrono::high_resolution_clock::time_point frameTime = std::chrono::high_resolution_clock::now();
	float deltaTime = 0;

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - frameTime).count();
			frameTime = std::chrono::high_resolution_clock::now();
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(vulkanBase->logicalDevice->device);
	}

	void cleanupSwapChain() {

		for (size_t i = 0; i < swapchain->getSize(); i++)
		{
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
		for (auto imageView : swapchain->swapChainImageViews) {
			vkDestroyImageView(vulkanBase->logicalDevice->device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(vulkanBase->logicalDevice->device, swapchain->handle, nullptr);

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			vkDestroyBuffer(vulkanBase->logicalDevice->device, uniformBuffers[i], nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, uniformBuffersMemory[i], nullptr);
		}

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			vkDestroyBuffer(vulkanBase->logicalDevice->device, shadingUniformBuffers[i], nullptr);
			vkFreeMemory(vulkanBase->logicalDevice->device, shadingUniformBuffersMemory[i], nullptr);
		}
		 */

		descriptorPool->destroy();
	}
	void cleanup() {
		cleanupSwapChain();

		//vkDestroySampler(vulkanBase->logicalDevice->device, textureSampler, nullptr);
		//vkDestroyImageView(vulkanBase->logicalDevice->device, textureImageView, nullptr);

		//vkDestroyImage(vulkanBase->logicalDevice->device, textureImage, nullptr);
		//vkFreeMemory(vulkanBase->logicalDevice->device, textureImageMemory, nullptr);

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
		createPipelines();
		//createAttachmentResources();
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

		//createAttachmentResources()
		swapchain->createAttachment(kAttachment_POSITION, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);	// (World space) Positions		
		swapchain->createAttachment(kAttachment_NORMAL, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);		// (World space) Normals		
		swapchain->createAttachment(kAttachment_ALBEDO, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		swapchain->createAttachment(kAttachment_DEPTH, vulkanBase->findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	
	void createRenderPass() {

		MyRenderPass* pass = new MyRenderPass(*vulkanBase, *swapchain);
		renderPass = std::unique_ptr<MyRenderPass>(pass);
				
	}

	void createFramebuffers() {
		swapchain->createFramebuffers(renderPass->renderPass);
	}

	void createDescriptorSetLayout() 
	{	
		//gbuf
		{
			SbPipelineLayout & DS = renderPass->getPipelineLayout(kSubpass_GBUF);

			//vertex
			{
				DS.addBufferBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
					*transformUniformBuffer);
			}
			//fragment
			{
				VkImageView  v = texture->textureImageView;
				DS.addImageBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
					{
						textureSampler,
						&v,//&textureImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						SbPipelineLayout::eSharingMode_Shared
					});
				DS.addBufferBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
					*shadingUniformBuffer);
			}
			//with bindings created, create layout
			DS.createDSLayout();
			DS.createPipelineLayout();
			//allocate descriptors and update them with resources
			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors();
		}

		//now create attachment read layout
		{
			//TODO simplify, combine with input attachments in renderpass?
			SbPipelineLayout & DS = renderPass->getPipelineLayout(kSubpass_COMPOSE);
			//frag
			{
				DS.addImageBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
					{
						VK_NULL_HANDLE,
						swapchain->getAttachmentViews(kAttachment_POSITION).data(),
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						SbPipelineLayout::eSharingMode_Separate
					});
				DS.addImageBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
					{
						VK_NULL_HANDLE,
						swapchain->getAttachmentViews(kAttachment_NORMAL).data(),
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						SbPipelineLayout::eSharingMode_Separate
					});
				DS.addImageBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
					{
						VK_NULL_HANDLE,
						swapchain->getAttachmentViews(kAttachment_ALBEDO).data(),
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						SbPipelineLayout::eSharingMode_Separate
					});
			}			
			//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			DS.createDSLayout();
			DS.createPipelineLayout();
			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors();
		}

		{
			SbPipelineLayout & DS = renderPass->getPipelineLayout(kSubpass_TRANSPARENT);
			//vertex
			{
				DS.addBufferBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
					*transformUniformBuffer);
			}
			//frag
			{
				VkImageView v = texture->textureImageView;
				DS.addImageBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
					{
						textureSampler,
						&v,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						SbPipelineLayout::eSharingMode_Shared
					});
				DS.addBufferBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
					*shadingUniformBuffer);
				DS.addImageBinding(
					vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
					{
						VK_NULL_HANDLE,
						swapchain->getAttachmentViews(kAttachment_POSITION).data(),
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						SbPipelineLayout::eSharingMode_Separate
					});
			}
			//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			DS.createDSLayout();
			DS.createPipelineLayout();
			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors();
			//todo simplify to pipelinelayout.create() +update descriptors?
		}


		
	}	

	void createPipelines() {
		//TODO why use index twice?
		auto & subpassgbuf = renderPass->subpasses[kSubpass_GBUF];
		const auto & bind = Vertex::getBindingDescriptions();
		const auto & attr = Vertex::getAttributeDescriptions();
		subpassgbuf.pipeline.subpassIndex(kSubpass_GBUF)
			.layout(subpassgbuf.pipelineLayout.pipelineLayout)
			.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE), 0, 3)
			.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
			.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})
			.addShaderStage(vks::helper::loadShader("shaders/gbuf.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, vulkanBase->logicalDevice->device))
			.addShaderStage(vks::helper::loadShader("shaders/gbuf.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, vulkanBase->logicalDevice->device))
			.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);


		auto & subpasscomp = renderPass->subpasses[kSubpass_COMPOSE];
		subpasscomp.pipeline.subpassIndex(kSubpass_COMPOSE)
			.layout(subpasscomp.pipelineLayout.pipelineLayout)
			.addShaderStage(vks::helper::loadShader("shaders/composition.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, vulkanBase->logicalDevice->device))
			.addShaderStage(vks::helper::loadShader("shaders/composition.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, vulkanBase->logicalDevice->device))
			.cullMode(VK_CULL_MODE_NONE)
			.depthWriteEnable(VK_FALSE)
			.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);

		auto & subpastransparent = renderPass->subpasses[kSubpass_TRANSPARENT];
		subpastransparent.pipeline.subpassIndex(kSubpass_TRANSPARENT)
			.layout(subpastransparent.pipelineLayout.pipelineLayout)
			.colorBlending(0)
			.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
			.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})
			.addShaderStage(vks::helper::loadShader("shaders/transparent.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, vulkanBase->logicalDevice->device))
			.addShaderStage(vks::helper::loadShader("shaders/transparent.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, vulkanBase->logicalDevice->device))
			.cullMode(VK_CULL_MODE_BACK_BIT)
			.depthWriteEnable(VK_FALSE)
			.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);
		
	}
	

		
	void createCommandPool() {
		vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);
	}
	
	

	

	void createTextureImage() {
		//TODO move loading to model loading. get model with textures
		texture = std::make_unique<SbTextureImage>(*vulkanBase, TEXTURE_PATH);
	}

	void createTextureSampler() {

		auto samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags(),
			vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, 
			vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
			0, VK_TRUE, 16, 
			VK_FALSE, vk::CompareOp::eAlways,
			0, texture->mipLevels, //todo this sampler is hardcoded to use the miplevels of the one and only texture
			vk::BorderColor::eIntOpaqueBlack,VK_FALSE);

		textureSampler = vulkanBase->getDevice().createSampler(samplerInfo);
	}
	
	Assimp::Importer modelImporter;
	const aiScene* modelScene;
	AnimationKeys running;
	AnimationKeys walking;
	//UncompressedAnimationKeys uk;
	//SkeletonAnimation sa()

	void loadModel() {

		running.loadAnimationData("running.chs");
		walking.loadAnimationData("walking.chs");
		//uk.loadAnimationData("uncompressed.chs");
		
		modelScene = modelImporter.ReadFile("jump.fbx",
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_ValidateDataStructure);

		std::map<std::string, uint64_t> jointLayout;
		auto rootJoint = AnimationStuff::findRootJoint(modelScene->mRootNode, modelScene);
		mymodel = new Model();
		AnimationStuff::makeFlatSkeleton(rootJoint, mymodel->skeleton, jointLayout, 0, -1, modelScene);
		AnimationStuff::finalizeFlatten(mymodel->skeleton);
		
		//SkeletonAnimation runningAnimation(mymodel->skeleton.jointCount, running); //todo get joint count from animation data instead?
		//SkeletonAnimation walkingAnimation(mymodel->skeleton.jointCount, walking); //todo get joint count from animation data instead?

		//UncompressedAnimation uncompAnimation(mymodel->skeleton.jointCount, uk);
		//jumpAnimation.plot(mymodel->skeleton);
		//uncompAnimation.plot(mymodel->skeleton);
		
		AnimationLayer baseLayer;
		baseLayer.blendAnimations.emplace_back(mymodel->skeleton.jointCount, walking);
		baseLayer.blendAnimations.emplace_back(mymodel->skeleton.jointCount, running);// todo interface

		mymodel->skeletalAnimationComponent.init(&mymodel->skeleton, 
			std::vector<AnimationLayer>{ baseLayer });
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
				size_t boneHierarchyIndex = jointLayout[currentMesh->mBones[j]->mName.C_Str()];
				for (size_t k = 0; k < currentMesh->mBones[j]->mNumWeights; k++) {
					size_t meshVertexID = currentMesh->mBones[j]->mWeights[k].mVertexId;
					float weight = currentMesh->mBones[j]->mWeights[k].mWeight;
					mymodel->meshes[i].vertexBuffer[meshVertexID].addBoneData(boneHierarchyIndex, weight);
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
		
	//TODO make function take model argument?
	void createVertexBuffer() {

		using BufferUsage = vk::BufferUsageFlagBits;
		using MemoryProperty = vk::MemoryPropertyFlagBits;

		for (size_t i = 0; i < mymodel->meshes.size(); i++)
		{
			vk::DeviceSize bufferSize = sizeof(Vertex) * mymodel->meshes[i].vertexBuffer.size();
			vk::DeviceSize indexBufferSize = sizeof(unsigned int) * mymodel->meshes[i].indexBuffer.size();

			//------
			SbBuffer stagingBuffer = SbBuffer(*vulkanBase ,bufferSize, BufferUsage::eTransferSrc,
				MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent);
			SbBuffer stagingIndexBuffer = SbBuffer(*vulkanBase, indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent);
			//------
			stagingBuffer.MapAndFill(vulkanBase->getDevice(), mymodel->meshes[i].vertexBuffer.data(), bufferSize);
			stagingIndexBuffer.MapAndFill(vulkanBase->getDevice(), mymodel->meshes[i].indexBuffer.data(), indexBufferSize);
			//-----
			SbBuffer newVertexBuffer = SbBuffer(*vulkanBase, bufferSize,
				BufferUsage::eTransferDst | BufferUsage::eVertexBuffer, MemoryProperty::eDeviceLocal);
			SbBuffer newIndexBuffer = SbBuffer(*vulkanBase, indexBufferSize,
				BufferUsage::eTransferDst | BufferUsage::eIndexBuffer, MemoryProperty::eDeviceLocal);
			//---- todo what should happen when new buffers go out of scope?
			drawables.emplace_back();
			drawables.back().id = drawables.size() - 1;
			drawables.back().VertexBuffer = newVertexBuffer.buffer;
			drawables.back().IndexBuffer = newIndexBuffer.buffer;
			drawables.back().VertexBufferMemory = newVertexBuffer.memory;
			drawables.back().IndexBufferMemory = newIndexBuffer.memory;
			drawables.back().IndexCount = mymodel->meshes[i].indexBuffer.size();
			drawables.back().AmbientColor = mymodel->meshes[i].material.ambientColor;
			drawables.back().DiffuseColor = mymodel->meshes[i].material.diffuseColor;
			drawables.back().SpecularColor = mymodel->meshes[i].material.specularColor;
			//vertexBuffers.push_back(newVertexBuffer);
			//vertexBufferMemory.push_back(newVertexBufferMemory);			
			//indexBuffer.push_back(newIndexBuffer);
			//indexBufferMemory.push_back(newIndexBufferMemory);
			//-----
			stagingBuffer.CopyBuffer(*vulkanBase, newVertexBuffer);
			stagingIndexBuffer.CopyBuffer(*vulkanBase, newIndexBuffer);
			
			stagingBuffer.Destroy(vulkanBase->getDevice());
			stagingIndexBuffer.Destroy(vulkanBase->getDevice());			
		}
		
	}

	void createUniformBuffers() {
		transformUniformBuffer = std::make_unique<SbUniformBuffer<UniformBufferObject>>(*vulkanBase, 
			swapchain->getSize()); 		

		skeletonUniformBuffer = std::make_unique<SbUniformBuffer<glm::mat4>>(
			*vulkanBase, swapchain->getSize(), 52*5); //room for 5 skeletons in buffer

		shadingUniformBuffer = std::make_unique<SbUniformBuffer<ShadingUBO>>(*vulkanBase, 1, drawables.size());

		for (size_t i = 0; i < drawables.size(); i++)
		{
			ShadingUBO data = { drawables[i].AmbientColor, drawables[i].DiffuseColor, drawables[i].SpecularColor };
			shadingUniformBuffer->writeBufferData(data, i);
		}
		shadingUniformBuffer->copyDataToBufferMemory(*vulkanBase);
	}

	void createDescriptorPool() {

		descriptorPool = std::make_unique<SbDescriptorPool>(vulkanBase->logicalDevice->device);

		std::vector<VkDescriptorPoolSize> poolSizes(4);
		poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapchain->getSize() * 2) }; //attachment write binding counts as uniform so there are 2 uniform bindings per frame, attachment and uniform struct
		poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapchain->getSize() * 2) };
		poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(swapchain->getSize() * 2) };
		poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(swapchain->getSize() * 4) };

		descriptorPool->createDescriptorPool(poolSizes, static_cast<uint32_t>(swapchain->getSize()) * 3);

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

			//todo clearvalues as properties of attachments, and attachments as part of renderpass
			std::array<VkClearValue, 5> clearValues = {};
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[3].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[4].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport(
				(float)swapchain->swapchainCI.imageExtent.width,
				(float)swapchain->swapchainCI.imageExtent.height, 0.0f, 1.0f);
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(swapchain->swapchainCI.imageExtent, 0, 0);
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			{
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_GBUF));



				for (size_t j = 0; j < drawables.size()-1; j++)
				{
					uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
					vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getPipelineLayout(kSubpass_GBUF).pipelineLayout,
						0, 1, &renderPass->getPipelineLayout(kSubpass_GBUF).allocatedDSs[i], 1, &offset);

					VkBuffer vert[] = { drawables[j].VertexBuffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);

					//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
					vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

					vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
				}
			}
			

			vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
			{

				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_COMPOSE));
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getPipelineLayout(kSubpass_COMPOSE).pipelineLayout, 
					0, 1, &renderPass->getPipelineLayout(kSubpass_COMPOSE).allocatedDSs[i], 0, NULL);
				vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			}


			vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
			{

				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_TRANSPARENT));

				for (size_t j = drawables.size()-1; j < drawables.size(); j++)
				{
					uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
					vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getPipelineLayout(kSubpass_TRANSPARENT).pipelineLayout,
						0, 1, &renderPass->getPipelineLayout(kSubpass_TRANSPARENT).allocatedDSs[i], 1, &offset);

					VkBuffer vert[] = { drawables[j].VertexBuffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);

					//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
					vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

					vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
				}
			}

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

		/*
		Skeleton& s = mymodel->skeleton;

		// the root has no parent
		s.localTransform[0] = AnimationStuff::makeAnimationMatrix(s.animationChannel[0], AnimationTime); //local animation transformation
		s.globalTransform[0] = s.localTransform[0];					//for root node local=global
		s.finalTransformation[0] = s.globalTransform[0] * s.offsetMatrix[0];


		for (unsigned int i = 1; i < mymodel->jointIndex.size(); ++i)
		{
			const uint16_t parentJointIndex = s.hierarchy[i];
			s.localTransform[i] = AnimationStuff::makeAnimationMatrix(s.animationChannel[i], AnimationTime); //local animation transformation
			s.globalTransform[i] = s.globalTransform[parentJointIndex] * s.localTransform[i]; //animation transform in space of the parent
			s.finalTransformation[i] = s.globalTransform[i] * s.offsetMatrix[i];
		}
		*/

		float s = 0.5f * std::sinf(time/(3.14f)) + 0.5f;
		mymodel->skeletalAnimationComponent.evaluate(deltaTime, std::vector<float> { s });

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) 
																//* glm::translate(glm::mat4(1.0f), { 0.0f,-1.0f,0.0f })
			* glm::scale(glm::mat4(1.0f), { 0.01f,0.01f ,0.01f });//* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
		ubo.view = cam.getViewMatrix();
		ubo.proj = cam.getProjectionMatrix();
		memcpy(ubo.boneTransforms, mymodel->skeletalAnimationComponent.transformations.data(), 
				sizeof(glm::mat4)*mymodel->skeletalAnimationComponent.transformations.size());
		transformUniformBuffer->writeBufferData(ubo);
		transformUniformBuffer->copyDataToBufferMemory(*vulkanBase, currentImage);

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

