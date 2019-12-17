

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

#include "SbVulkanBase.h"

#include "SbLayout.h"
#include "SbDescriptorPool.h"
#include "SbDescriptorSets.h"

#include "SbSwapchain.h"



namespace vkinit = vks::initializers;

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;

//TODO transition to vulkanbase
/*c
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	//,"VK_LAYER_LUNARG_api_dump"
};
*/

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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
	//todo transition to vulkanbase
	/*
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	*/

	std::unique_ptr<SbPhysicalDevice> physicalDevice;
	/*
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	*/
	
	std::unique_ptr<SbLogicalDevice> logicalDevice;
	/*
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	*/

	std::unique_ptr<SbSwapchain> swapchain;
	/*
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	*/

	VkRenderPass renderPass;

	struct SubpassSetup {
		//std::unique_ptr<SbLayout> layout;
		std::unique_ptr<SbDescriptorSets> descriptorSets;
		//VkDescriptorSetLayout DS_Layout;
		//std::vector<VkDescriptorSet> allocatedDSs;
		//VkPipelineLayout Pipeline_Layout;
		VkPipeline Pipeline;
	};

	SubpassSetup attachmentWriteSubpass;
	SubpassSetup attachmentReadSubpass;

	//VkPipeline graphicsPipeline;

	VkCommandPool commandPool;

	


	enum
	{
		kAttachment_BACK,
		kAttachment_COLOR,
		kAttachment_DEPTH,
		kAttachment_COUNT,
		kAttachment_MAX = kAttachment_DEPTH
	};
	enum
	{
		kSubpass_GBUF = 0,
		kSubpass_COMPOSE = 1
	};


	//todo transition attachments to swapchain?
	/*
	struct AttachmentSet {
		std::vector<VkImage> image;
		std::vector<VkDeviceMemory> mem;
		std::vector<VkImageView> view;
		//VkFormat format;
		//VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

		AttachmentSet(size_t size)
			: image(size), mem(size), view(size) 
			{	}
	};

	enum attachmentIndex { eSetIndex_Color, eSetIndex_Depth, eSetIndex_COUNT, eSetIndex_MAX = eSetIndex_Depth};
	std::vector<AttachmentSet> attachmentSets;
	*/

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	//custom model struct
	Model* mymodel = nullptr;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	std::vector<VkBuffer> shadingUniformBuffers;
	std::vector<VkDeviceMemory> shadingUniformBuffersMemory;

	std::vector<VkBuffer> transformationgUB;
	std::vector<VkDeviceMemory> transformationUBMemory;

	//VkDescriptorPool descriptorPool;
	std::unique_ptr<SbDescriptorPool> descriptorPool;

	//std::vector<VkDescriptorSet> descriptorSets;

	/*struct {
		std::vector <VkDescriptorSet> attachmentWrite;
		std::vector<VkDescriptorSet> attachmentRead;
	} descriptorSets;*/

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	size_t dynamicAlignment;
	size_t dynamicBufferSize;

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
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();

		createSwapChain();
		createAttachmentResources();

		createRenderPass();
		createFramebuffers();

		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		loadModel();
		createVertexBuffer();
		//createIndexBuffer();

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

		vkDeviceWaitIdle(logicalDevice->device);
	}

	void cleanupSwapChain() {

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++)
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
			vkDestroyFramebuffer(logicalDevice->device, framebuffer, nullptr);
		}

		vkFreeCommandBuffers(logicalDevice->device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(logicalDevice->device, attachmentWriteSubpass.Pipeline, nullptr);
		vkDestroyPipeline(logicalDevice->device, attachmentReadSubpass.Pipeline, nullptr);
		vkDestroyPipelineLayout(logicalDevice->device, attachmentWriteSubpass.descriptorSets.get()->pipelineLayout, nullptr);
		vkDestroyPipelineLayout(logicalDevice->device, attachmentReadSubpass.descriptorSets.get()->pipelineLayout, nullptr);
		vkDestroyRenderPass(logicalDevice->device, renderPass, nullptr);

		for (auto imageView : swapchain->swapChainImageViews) {
			vkDestroyImageView(logicalDevice->device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(logicalDevice->device, swapchain->handle, nullptr);

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			vkDestroyBuffer(logicalDevice->device, uniformBuffers[i], nullptr);
			vkFreeMemory(logicalDevice->device, uniformBuffersMemory[i], nullptr);
		}

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			vkDestroyBuffer(logicalDevice->device, shadingUniformBuffers[i], nullptr);
			vkFreeMemory(logicalDevice->device, shadingUniformBuffersMemory[i], nullptr);
		}

		descriptorPool->destroy();
	}

	void cleanup() {
		cleanupSwapChain();

		vkDestroySampler(logicalDevice->device, textureSampler, nullptr);
		vkDestroyImageView(logicalDevice->device, textureImageView, nullptr);

		vkDestroyImage(logicalDevice->device, textureImage, nullptr);
		vkFreeMemory(logicalDevice->device, textureImageMemory, nullptr);

		vkDestroyDescriptorSetLayout(logicalDevice->device, attachmentWriteSubpass.descriptorSets.get()->DSLayout, nullptr);
		vkDestroyDescriptorSetLayout(logicalDevice->device, attachmentReadSubpass.descriptorSets.get()->DSLayout, nullptr);

		for (size_t i = 0; i < drawables.size(); i++)
		{
			vkDestroyBuffer(logicalDevice->device, drawables[i].IndexBuffer, nullptr);
			vkFreeMemory(logicalDevice->device, drawables[i].IndexBufferMemory, nullptr);

			vkDestroyBuffer(logicalDevice->device, drawables[i].VertexBuffer, nullptr);
			vkFreeMemory(logicalDevice->device, drawables[i].VertexBufferMemory, nullptr);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicalDevice->device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(logicalDevice->device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(logicalDevice->device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(logicalDevice->device, commandPool, nullptr);

		vkDestroyDevice(logicalDevice->device, nullptr);

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

		vkDeviceWaitIdle(logicalDevice->device);

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
	}

	void pickPhysicalDevice() {
		physicalDevice = std::make_unique<SbPhysicalDevice>(vulkanBase->instance, vulkanBase->surface);
	}

	void createLogicalDevice() {
		logicalDevice = std::make_unique<SbLogicalDevice>(*physicalDevice);
		logicalDevice->createLogicalDevice(vulkanBase->surface, enableValidationLayers, validationLayers);		
	}

	void createSwapChain() {
		swapchain = std::make_unique<SbSwapchain>(*physicalDevice, *logicalDevice);
		swapchain->createSwapChain(vulkanBase->surface, window);
		swapchain->createImageViews(logicalDevice->device);	
	}
	
	void createRenderPass() {
		//willems attachments demo
		std::array<VkAttachmentDescription, kAttachment_COUNT> attachments {};
		attachments[kAttachment_BACK] = swapchain->swapchainAttachmentDescription;
		attachments[kAttachment_COLOR] = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Color].description;
		attachments[kAttachment_DEPTH] = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Depth].description;

		/*
		// Swap chain image color attachment
		// Will be transitioned to present layout
		attachments[0].format = swapchain->swapChainImageFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Input attachments
		// These will be written in the first subpass, transitioned to input attachments 
		// and then read in the secod subpass

		// Color
		attachments[1].format = swapchain->swapChainImageFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// Depth
		attachments[2].format = findDepthFormat();
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		*/

		std::array<VkSubpassDescription, 2> subpassDescriptions {};

		/*
			First subpass
			Fill the color and depth attachments
		*/
		VkAttachmentReference colorReference = { kAttachment_COLOR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depthReference = { kAttachment_DEPTH, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[0].colorAttachmentCount = 1;
		subpassDescriptions[0].pColorAttachments = &colorReference;
		subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

		/*
			Second subpass
			Input attachment read and swap chain color attachment write
		*/

		// Color and depth attachment written to in first sub pass will be used as input attachments to be read in the fragment shader
		VkAttachmentReference inputReferences[2];
		inputReferences[0] = { kAttachment_COLOR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[1] = { kAttachment_DEPTH, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		// Color reference (target) for this sub pass is the swap chain color attachment
		VkAttachmentReference colorReferenceSwapchain = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[1].colorAttachmentCount = 1;
		subpassDescriptions[1].pColorAttachments = &colorReferenceSwapchain;
		// Use the attachments filled in the first pass as input attachments
		subpassDescriptions[1].inputAttachmentCount = 2;
		subpassDescriptions[1].pInputAttachments = inputReferences;

		/*
			Subpass dependencies for layout transitions
		*/
		std::array<VkSubpassDependency, 3> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// This dependency transitions the input attachment from color attachment to shader read
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = 1;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[2].srcSubpass = 1;
		dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(logicalDevice->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void createDescriptorSetLayout() {
		//first create attachment write layout
		{
			attachmentWriteSubpass.descriptorSets = std::make_unique<SbDescriptorSets>(logicalDevice->device, swapchain->swapChainImages.size());
			SbDescriptorSets & DS = *attachmentWriteSubpass.descriptorSets.get();
			
			//create bindings
			DS.addBufferBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0), 
				{ 
					uniformBuffers.data(),
					0, 
					sizeof(UniformBufferObject), 
					SbDescriptorSets::eBindingMode_Separate
				});
			DS.addImageBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1), 
				{ 
					textureSampler, 
					&textureImageView, 
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					SbDescriptorSets::eBindingMode_Shared 
				});
			DS.addBufferBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2), 
				{ 
					shadingUniformBuffers.data(),
					0, 
					static_cast<uint64_t>(dynamicAlignment), 
					SbDescriptorSets::eBindingMode_Separate 
				});
			
			//with bindings created, create layout
			DS.createDSLayout();
			DS.createPipelineLayout();

			//allocate descriptors and update them with resources
			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors();
		}

		//now create attachment read layout
		{
			attachmentReadSubpass.descriptorSets = std::make_unique<SbDescriptorSets>(logicalDevice->device, swapchain->swapChainImages.size());
			SbDescriptorSets & DS = *attachmentReadSubpass.descriptorSets.get();

			DS.addImageBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
				{ 
					VK_NULL_HANDLE, 
					swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Color].view.data(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					SbDescriptorSets::eBindingMode_Separate 
				});
			DS.addImageBinding(
				vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
				{
					VK_NULL_HANDLE,
					swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Depth].view.data(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
					SbDescriptorSets::eBindingMode_Separate 
				});
			//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			DS.createDSLayout();
			DS.createPipelineLayout();

			DS.allocateDescriptorSets(*descriptorPool.get());
			DS.updateDescriptors(); 

		}
	}	

	void createDoublePipeline() {
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationStateCI = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS); 
		VkPipelineViewportStateCreateInfo viewportStateCI = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleStateCI = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo();

		pipelineCI.renderPass = renderPass;
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCI.pStages = shaderStages.data();

		/*
			Attachment write
		*/

		// Pipeline will be used in first sub pass
		pipelineCI.subpass = kSubpass_GBUF;
		pipelineCI.layout = attachmentWriteSubpass.descriptorSets->pipelineLayout;

		// Binding description
		auto vertexInputBindings = Vertex::getBindingDescriptions();

		// Attribute descriptions
		auto vertexInputAttributes = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI = vks::initializers::pipelineVertexInputStateCreateInfo();
		vertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputStateCI.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCI.pVertexInputState = &vertexInputStateCI;

		shaderStages[0] = vks::helper::loadShader("shaders/attachmentwrite.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, logicalDevice->device);
		shaderStages[1] = vks::helper::loadShader("shaders/attachmentwrite.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, logicalDevice->device);
		vkCreateGraphicsPipelines(logicalDevice->device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &attachmentWriteSubpass.Pipeline);

		/*
			Attachment read
		*/

		// Pipeline will be used in second sub pass
		pipelineCI.subpass = kSubpass_COMPOSE;
		pipelineCI.layout = attachmentReadSubpass.descriptorSets->pipelineLayout;

		VkPipelineVertexInputStateCreateInfo emptyInputStateCI {};
		emptyInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		pipelineCI.pVertexInputState = &emptyInputStateCI;
		colorBlendStateCI.attachmentCount = 1;
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		depthStencilStateCI.depthWriteEnable = VK_FALSE;

		shaderStages[0] = vks::helper::loadShader("shaders/attachmentread.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, logicalDevice->device);
		shaderStages[1] = vks::helper::loadShader("shaders/attachmentread.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, logicalDevice->device);
		vkCreateGraphicsPipelines(logicalDevice->device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &attachmentReadSubpass.Pipeline);
		
	}
	
	void createFramebuffers() {
		swapchain->swapChainFramebuffers.resize(swapchain->swapChainImageViews.size());


		std::array<VkImageView, 3> attachmentViews = {};

		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderPass;
		framebufferCI.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
		framebufferCI.pAttachments = attachmentViews.data();
		framebufferCI.width = swapchain->swapChainExtent.width;
		framebufferCI.height = swapchain->swapChainExtent.height;
		framebufferCI.layers = 1;

		for (size_t i = 0; i < swapchain->swapChainImageViews.size(); i++) {			
			
			attachmentViews[kAttachment_BACK] = swapchain->swapChainImageViews[i];
			attachmentViews[kAttachment_COLOR] = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Color].view[i]; //attachments[i].color.view;
			attachmentViews[kAttachment_DEPTH] = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Depth].view[i];

			if (vkCreateFramebuffer(logicalDevice->device, &framebufferCI, nullptr, &swapchain->swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void createCommandPool() {
		//QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
		QueueFamilyIndices queueFamilyIndices = physicalDevice->findQueueFamilies(physicalDevice->device, vulkanBase->surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(logicalDevice->device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}

	void createAttachmentResources() {
		const SbSwapchain::SwapchainAttachment as(swapchain->swapChainImages.size());
		swapchain->swapchainAttachmentSets = 
			std::vector<SbSwapchain::SwapchainAttachment>(SbSwapchain::attachmentIndex::eSetIndex_COUNT, as);		
		// Input attachments
		// These will be written in the first subpass, transitioned to input attachments 
		// and then read in the secod subpass

		// Color
		auto & colorAttachment = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Color].description;
		colorAttachment.format = swapchain->swapchainAttachmentDescription.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		// Depth
		auto & depthAttachment = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Depth].description;
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		
		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++)
		{
			{
				auto& attachment = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Color];
				auto& format = attachment.description.format;
				auto& samples = attachment.description.samples;
				auto& image = attachment.image[i];
				auto& memory = attachment.mem[i];
				auto& view = attachment.view[i];
				format = swapchain->swapchainAttachmentDescription.format;
				createImage(swapchain->swapChainExtent.width, swapchain->swapChainExtent.height, 1, samples, format, VK_IMAGE_TILING_OPTIMAL, 
					VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
				view = vks::helper::createImageView(logicalDevice->device, image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
				transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
			}
			{
				auto& attachment = swapchain->swapchainAttachmentSets[SbSwapchain::attachmentIndex::eSetIndex_Depth];
				auto& format =	attachment.description.format;
				auto& samples = attachment.description.samples;
				auto& image =	attachment.image[i];
				auto& memory =	attachment.mem[i];
				auto& view =	attachment.view[i];
				format = findDepthFormat();
				createImage(swapchain->swapChainExtent.width, swapchain->swapChainExtent.height, 1, samples, format,
					VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
				view = vks::helper::createImageView(logicalDevice->device, image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
				transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
			}
		}

		/*colorAttachment.format = swapChainImageFormat;
		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorAttachment.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorAttachment.image, colorAttachment.mem);
		colorAttachment.view = createImageView(colorAttachment.image, colorAttachment.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(colorAttachment.image, colorAttachment.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);


		normalsAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, normalsAttachment.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalsAttachment.image, normalsAttachment.mem);
		normalsAttachment.view = createImageView(normalsAttachment.image, normalsAttachment.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(normalsAttachment.image, normalsAttachment.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
		
		positionsAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, positionsAttachment.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, positionsAttachment.image, positionsAttachment.mem);
		positionsAttachment.view = createImageView(positionsAttachment.image, positionsAttachment.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		transitionImageLayout(positionsAttachment.image, positionsAttachment.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
		*/
	}

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice->device, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat findDepthFormat() {
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDevice->device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(logicalDevice->device, stagingBufferMemory);

		stbi_image_free(pixels);

		createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		vkDestroyBuffer(logicalDevice->device, stagingBuffer, nullptr);
		vkFreeMemory(logicalDevice->device, stagingBufferMemory, nullptr);

		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);
	}

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice->device, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}

	void createTextureImageView() {
		textureImageView = vks::helper::createImageView(logicalDevice->device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
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

		if (vkCreateSampler(logicalDevice->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(logicalDevice->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicalDevice->device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(logicalDevice->device, image, imageMemory, 0);
	}

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
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
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			VkBuffer stagingIndexBuffer;
			VkDeviceMemory stagingIndexBufferMemory;
			createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);

			//------
			void* data;
			vkMapMemory(logicalDevice->device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, mymodel->meshes[i].vertexBuffer.data(), (size_t)bufferSize);
			vkUnmapMemory(logicalDevice->device, stagingBufferMemory);


			void* indexdata;
			vkMapMemory(logicalDevice->device, stagingIndexBufferMemory, 0, indexBufferSize, 0, &indexdata);
			memcpy(indexdata, mymodel->meshes[i].indexBuffer.data(), (size_t)indexBufferSize);
			vkUnmapMemory(logicalDevice->device, stagingIndexBufferMemory);
			//-----
			VkBuffer newVertexBuffer;
			VkDeviceMemory newVertexBufferMemory;
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newVertexBuffer, newVertexBufferMemory);

			VkBuffer newIndexBuffer;
			VkDeviceMemory newIndexBufferMemory;
			createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newIndexBuffer, newIndexBufferMemory);
			
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

			copyBuffer(stagingBuffer, drawables.back().VertexBuffer, bufferSize);
			copyBuffer(stagingIndexBuffer, drawables.back().IndexBuffer, indexBufferSize);
			//----
			vkDestroyBuffer(logicalDevice->device, stagingBuffer, nullptr);
			vkFreeMemory(logicalDevice->device, stagingBufferMemory, nullptr);

			vkDestroyBuffer(logicalDevice->device, stagingIndexBuffer, nullptr);
			vkFreeMemory(logicalDevice->device, stagingIndexBufferMemory, nullptr);
		}
		
	}

	void createUniformBuffers() {
		// Allocate data for the dynamic uniform buffer object
		// We allocate this manually as the alignment of the offset differs between GPUs

		// Calculate required alignment based on minimum device offset alignment
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice->device, &physicalDeviceProperties);

		size_t minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
		dynamicAlignment = sizeof(ShadingUBO);
		if (minUboAlignment > 0) {
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}

		dynamicBufferSize = drawables.size() * dynamicAlignment;

		shadingUboData = (ShadingUBO*)alignedAlloc(dynamicBufferSize, dynamicAlignment);
		assert(shadingUboData);

		for (size_t i = 0; i < drawables.size(); i++)
		{
			ShadingUBO* shubo = (ShadingUBO*)(((uint64_t)shadingUboData + (i * dynamicAlignment)));
			shubo->ambientColor = drawables[i].AmbientColor;
			shubo->diffuseColor = drawables[i].DiffuseColor;
			shubo->specularColor = drawables[i].SpecularColor;
		}

		shadingUniformBuffers.resize(swapchain->swapChainImages.size());
		shadingUniformBuffersMemory.resize(swapchain->swapChainImages.size());

		//------------------------------

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(swapchain->swapChainImages.size());
		uniformBuffersMemory.resize(swapchain->swapChainImages.size());

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
			createBuffer(dynamicBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, shadingUniformBuffers[i], shadingUniformBuffersMemory[i]);
		}

		for (size_t i = 0; i < shadingUniformBuffersMemory.size(); i++)
		{
			void* data;
			vkMapMemory(logicalDevice->device, shadingUniformBuffersMemory[i], 0, dynamicBufferSize, 0, &data);
			memcpy(data, shadingUboData, dynamicBufferSize);
			vkUnmapMemory(logicalDevice->device, shadingUniformBuffersMemory[i]);
		}
	}

	void createDescriptorPool() {

		descriptorPool = std::make_unique<SbDescriptorPool>(logicalDevice->device);

		std::vector<VkDescriptorPoolSize> poolSizes(4);
		poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapchain->swapChainImages.size() * 2) }; //attachment write binding counts as uniform so there are 2 uniform bindings per frame, attachment and uniform struct
		poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(swapchain->swapChainImages.size() * 2) };
		poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(swapchain->swapChainImages.size() * 2) };
		poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(swapchain->swapChainImages.size() * 2) };


		descriptorPool->createDescriptorPool(poolSizes, static_cast<uint32_t>(swapchain->swapChainImages.size()) * 2);

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

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(logicalDevice->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice->device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(logicalDevice->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(logicalDevice->device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(logicalDevice->device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(logicalDevice->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(logicalDevice->graphicsQueue);

		vkFreeCommandBuffers(logicalDevice->device, commandPool, 1, &commandBuffer);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice->device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void createCommandBuffers() {
		commandBuffers.resize(swapchain->swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(logicalDevice->device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
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
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapchain->swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapchain->swapChainExtent;

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

			VkViewport viewport = vks::initializers::viewport((float)swapchain->swapChainExtent.width, (float)swapchain->swapChainExtent.height, 0.0f, 1.0f);
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(swapchain->swapChainExtent, 0, 0);
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, attachmentWriteSubpass.Pipeline);

			for (size_t j = 0; j < drawables.size(); j++)
			{
				uint32_t offset = j * static_cast<uint32_t>(dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, attachmentWriteSubpass.descriptorSets->pipelineLayout,
					0, 1, &attachmentWriteSubpass.descriptorSets->allocatedDSs[i], 1, &offset);

				VkBuffer vert[] = { drawables[j].VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);

				//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
			}

			vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, attachmentReadSubpass.Pipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, attachmentReadSubpass.descriptorSets->pipelineLayout, 0, 1, &attachmentReadSubpass.descriptorSets->allocatedDSs[i], 0, NULL);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(logicalDevice->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(logicalDevice->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(logicalDevice->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
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

		void* data;
		vkMapMemory(logicalDevice->device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(logicalDevice->device, uniformBuffersMemory[currentImage]);

	}

	void drawFrame() {
		VkResult res = vkWaitForFences(logicalDevice->device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		if (res != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(logicalDevice->device, swapchain->handle, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		updateUniformBuffer(imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(logicalDevice->device, 1, &inFlightFences[currentFrame]);

		res = vkQueueSubmit(logicalDevice->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
		if (res != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapchain->handle };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(logicalDevice->presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

