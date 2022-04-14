#include "HelloTriangleApplication.h"

#include "VulkanHelperFunctions.hpp"
#include "VulkanInitializers.hpp"

#include "SbDescriptorPool.h"
//#include "SbDescriptorSet.h"
#include "MyRenderPass.h"
#include "SbTextureImage.h"
#include "SbImage.h"

#include "ResourceManager.h"

#include "Sponza.h"

#include "RenderpassHelper.h"

Sponza sponza;

// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment)
{
	void* data = nullptr;
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


HelloTriangleApplication::HelloTriangleApplication()
	:cam(WIDTH, HEIGHT)
{	}

void HelloTriangleApplication::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

HelloTriangleApplication::~HelloTriangleApplication() {
	delete(mymodel);
}

glm::dvec2 HelloTriangleApplication::last_mouse_pos;
glm::dvec2 HelloTriangleApplication::delta_mouse_pos = glm::dvec2();
std::vector<int> HelloTriangleApplication::keyPresses;

void HelloTriangleApplication::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowUserPointer(window, this);

	glfwSetCursorPosCallback(window, mouse_callback);


	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	

	assert(glfwRawMouseMotionSupported());
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	
	glfwGetCursorPos(window, &last_mouse_pos.x, &last_mouse_pos.y);	

	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}



void HelloTriangleApplication::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		keyPresses.push_back(key);
	}


	std::cout << "basic key callback" << std::endl;
}

void HelloTriangleApplication::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{	
	glm::dvec2 new_mouse_pos(xpos,ypos);	
	delta_mouse_pos += new_mouse_pos - last_mouse_pos;	
	last_mouse_pos = new_mouse_pos;	
}



void HelloTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void HelloTriangleApplication::initVulkan() {
	createInstance();
	vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = vulkanBase->getPhysicalDevice();
	allocatorInfo.device = vulkanBase->getDevice();
	allocatorInfo.instance = vulkanBase->instance;
	vmaCreateAllocator(&allocatorInfo, &vmaAllocator);

	ResourceManager::getInstance().vkBase = vulkanBase.get(); //todo ugly bad



	sponza.load(vulkanBase.get());



	swapchain = new SbSwapchain(*vulkanBase);
	swapchain->createSwapChain(vulkanBase->surface, window);

	//MyRenderPass* pass = new MyRenderPass(*vulkanBase, *swapchain);
	//renderPass = pass;
	createDeferredRenderpass();

	//swapchain->createFramebuffersForRenderpass(renderPass->renderPass);

	//todo why is this here and why does it fail?
	//texture = std::make_unique<SbTextureImage>(*vulkanBase, TEXTURE_PATH); 


	createTextureSampler();

	mymodel = new AnimatedModel("jump.fbx");
	createVertexBuffer();

	createUniformBuffers();



	createDescriptorPool();

	createPipelines();
	createDescriptorSets();

	createCommandBuffers();
	swapchain->createSyncObjects(MAX_FRAMES_IN_FLIGHT);
}

void HelloTriangleApplication::createShadowRenderpass() {
	VkDevice device = vulkanBase->getDevice();
	enum subpasses {		
		sDEPTH,
		sCOUNT
	};

	enum attachments {		
		aDEPTH,
		aCOUNT
	};
	
	shadowRenderPass = new RenderpassHelper(sCOUNT, aCOUNT);
	shadowRenderPass->depthAttachmentDesc(aDEPTH, vulkanBase->findDepthFormat());
	shadowRenderPass->setDepthStencilAttachmentRef(aDEPTH, sDEPTH);
	shadowRenderPass->createRenderpass(device);

	shadowFrameBuffers.resize(swapchain->getSize());
	for (size_t i = 0; i < swapchain->getSize(); i++)
	{
		shadowFrameBuffers[i].createAttachmentImage(vulkanBase.get(), *shadowRenderPass, aDEPTH);
		shadowFrameBuffers[i].createFramebuffer(device);
	}
}

void HelloTriangleApplication::createDeferredRenderpass() 
{
	VkDevice device = vulkanBase->getDevice();

	enum subpasses {
		sGBUF,
		sCOMPOSE,
		sCOUNT
	};

	enum attachments {
		aBACK, 
		aPOSITION, 
		aNORMAL,
		aALBEDO,
		aDEPTH,
		aCOUNT
	};

	deferredRenderPass = new RenderpassHelper(sCOUNT, aCOUNT);
	deferredRenderPass->colorAttachmentDesc(aPOSITION, VK_FORMAT_R16G16B16A16_SFLOAT);
	deferredRenderPass->colorAttachmentDesc(aNORMAL, VK_FORMAT_R16G16B16A16_SFLOAT);
	deferredRenderPass->colorAttachmentDesc(aALBEDO, VK_FORMAT_R8G8B8A8_UNORM);
	deferredRenderPass->depthAttachmentDesc(aDEPTH, vulkanBase->findDepthFormat());

	deferredRenderPass->addColorAttachmentRef(sGBUF, aPOSITION);
	deferredRenderPass->addColorAttachmentRef(sGBUF, aNORMAL);
	deferredRenderPass->addColorAttachmentRef(sGBUF, aALBEDO);
	deferredRenderPass->setDepthStencilAttachmentRef(sGBUF, aDEPTH);

	deferredRenderPass->addColorAttachmentRef(sCOMPOSE, aBACK);
	deferredRenderPass->addInputAttachmentRef(sCOMPOSE, aPOSITION);
	deferredRenderPass->addInputAttachmentRef(sCOMPOSE, aNORMAL);
	deferredRenderPass->addInputAttachmentRef(sCOMPOSE, aALBEDO);
	deferredRenderPass->setDepthStencilAttachmentRef(sCOMPOSE, aDEPTH);
	
	std::array<VkSubpassDependency, 3> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = sGBUF;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This dependency transitions the input attachment from color attachment to shader read
	dependencies[1].srcSubpass = sGBUF;
	dependencies[1].dstSubpass = sCOMPOSE;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
	dependencies[2].srcSubpass = sCOMPOSE;
	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	deferredRenderPass->addDependency(dependencies[0]);
	deferredRenderPass->addDependency(dependencies[1]);
	deferredRenderPass->addDependency(dependencies[2]);

	deferredRenderPass->createRenderpass(device);

	deferredFrameBuffers.resize(swapchain->getSize());

	for (size_t i = 0; i < swapchain->getSize(); i++)
	{
		deferredFrameBuffers[i].addAttachmentImage(aBACK, swapchain->swapChainImageViews[i]);
		deferredFrameBuffers[i].createAttachmentImage(vulkanBase.get(), *deferredRenderPass, aPOSITION);
		deferredFrameBuffers[i].createAttachmentImage(vulkanBase.get(), *deferredRenderPass, aNORMAL);
		deferredFrameBuffers[i].createAttachmentImage(vulkanBase.get(), *deferredRenderPass, aALBEDO);
		deferredFrameBuffers[i].createAttachmentImage(vulkanBase.get(), *deferredRenderPass, aDEPTH);
		deferredFrameBuffers[i].createFramebuffer(device);
	}
	
}


void HelloTriangleApplication::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - frameTime).count();
		frameTime = std::chrono::high_resolution_clock::now();
		glfwPollEvents();
		handleKeyPresses();
		drawFrame();
		delta_mouse_pos = { 0, 0 }; //?????
	}

	vkDeviceWaitIdle(vulkanBase->getDevice());
}

void HelloTriangleApplication::cleanupSwapChain() {

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

	//for (auto framebuffer : swapchain->swapChainFramebuffers) {
	//	vkDestroyFramebuffer(vulkanBase->getDevice(), framebuffer, nullptr);
	//}

	vkFreeCommandBuffers(vulkanBase->getDevice(), vulkanBase->commandPool->handle, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	//vkDestroyPipeline(vulkanBase->getDevice(), attachmentWriteSubpass.Pipeline, nullptr);
	//vkDestroyPipeline(vulkanBase->getDevice(), attachmentReadSubpass.Pipeline, nullptr);
	//vkDestroyPipelineLayout(vulkanBase->getDevice(), attachmentWriteSubpass.descriptorSets.get()->pipelineLayout, nullptr);
	//vkDestroyPipelineLayout(vulkanBase->getDevice(), attachmentReadSubpass.descriptorSets.get()->pipelineLayout, nullptr);
	//vkDestroyRenderPass(vulkanBase->getDevice(), renderPass->renderPass, nullptr);

	/*
	for (auto imageView : swapchain->swapChainImageViews) {
	vkDestroyImageView(vulkanBase->getDevice(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(vulkanBase->getDevice(), swapchain->handle, nullptr);

	for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
	vkDestroyBuffer(vulkanBase->getDevice(), uniformBuffers[i], nullptr);
	vkFreeMemory(vulkanBase->getDevice(), uniformBuffersMemory[i], nullptr);
	}

	for (size_t i = 0; i < swapchain->swapChainImages.size(); i++) {
	vkDestroyBuffer(vulkanBase->getDevice(), shadingUniformBuffers[i], nullptr);
	vkFreeMemory(vulkanBase->getDevice(), shadingUniformBuffersMemory[i], nullptr);
	}
	*/

	descriptorPool->destroy();
}

void HelloTriangleApplication::cleanup() {
	cleanupSwapChain();

	//vkDestroySampler(vulkanBase->getDevice(), textureSampler, nullptr);
	//vkDestroyImageView(vulkanBase->getDevice(), textureImageView, nullptr);

	//vkDestroyImage(vulkanBase->getDevice(), textureImage, nullptr);
	//vkFreeMemory(vulkanBase->getDevice(), textureImageMemory, nullptr);

	//vkDestroyDescriptorSetLayout(vulkanBase->getDevice(), attachmentWriteSubpass.descriptorSets.get()->DSLayout, nullptr);
	//vkDestroyDescriptorSetLayout(vulkanBase->getDevice(), attachmentReadSubpass.descriptorSets.get()->DSLayout, nullptr);

	for (size_t i = 0; i < drawables.size(); i++)
	{
		vkDestroyBuffer(vulkanBase->getDevice(), drawables[i].IndexBuffer, nullptr);
		vkFreeMemory(vulkanBase->getDevice(), drawables[i].IndexBufferMemory, nullptr);

		vkDestroyBuffer(vulkanBase->getDevice(), drawables[i].VertexBuffer, nullptr);
		vkFreeMemory(vulkanBase->getDevice(), drawables[i].VertexBufferMemory, nullptr);
	}

	/*
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
	vkDestroySemaphore(vulkanBase->getDevice(), renderFinishedSemaphores[i], nullptr);
	vkDestroySemaphore(vulkanBase->getDevice(), imageAvailableSemaphores[i], nullptr);
	vkDestroyFence(vulkanBase->getDevice(), inFlightFences[i], nullptr);
	}
	*/

	vkDestroyCommandPool(vulkanBase->getDevice(), vulkanBase->commandPool->handle, nullptr);

	vkDestroyDevice(vulkanBase->getDevice(), nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vulkanBase->instance, vulkanBase->debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(vulkanBase->instance, vulkanBase->surface, nullptr);
	vkDestroyInstance(vulkanBase->instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

void HelloTriangleApplication::recreateSwapChain() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(vulkanBase->getDevice());

	cleanupSwapChain();

	//createSwapChain();
	//createImageViews(); moved to swapchain creation
	//createRenderPass();
	createPipelines();
	//createAttachmentResources();
	//createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	//createDescriptorSets();
	createCommandBuffers();
}

void HelloTriangleApplication::createInstance() {
	vulkanBase = std::make_unique<SbVulkanBase>();
	vulkanBase->createInstance(enableValidationLayers);
	vulkanBase->setupDebugMessenger(enableValidationLayers);//moved here from setupdebugmessenger
	vulkanBase->createSurface(window); //moved from createsurface

	vulkanBase->pickPhysicalDevice();
	vulkanBase->createLogicalDevice();
}


void HelloTriangleApplication::handleKeyPresses()
{

	for (size_t i = 0; i < keyPresses.size(); i++)
	{
		switch (keyPresses[i])
		{
		case GLFW_KEY_0:
			setOutputMode(0);
			goto break_out;
		case GLFW_KEY_1:
			setOutputMode(1);
			goto break_out;
		case GLFW_KEY_2:
			setOutputMode(2);
			goto break_out;
		case GLFW_KEY_3:
			setOutputMode(3);
			goto break_out;
		default:
			break;
		}
	}
break_out:

	keyPresses.clear();

	

}

void HelloTriangleApplication::setOutputMode(uint32_t mode)
{
	struct SpecializationData {
		uint32_t output_mode;
	} specializationData;

	specializationData.output_mode = mode;


	VkSpecializationMapEntry entry = vk::SpecializationMapEntry();
	entry.constantID = 0;
	entry.offset = 0;
	entry.size = sizeof(specializationData.output_mode);

	VkSpecializationInfo specialization = vk::SpecializationInfo();
	specialization.mapEntryCount = 1;
	specialization.pMapEntries = &entry;
	specialization.pData = &specializationData;
	specialization.dataSize = sizeof(specializationData);

	pipelines.composition.specializeFrag(&specialization).createPipeline(deferredRenderPass->renderPass, vulkanBase->getDevice());

	//rebuild command buffers
	createCommandBuffers();
}

void HelloTriangleApplication::createPipelines()
{
	VkDevice device = vulkanBase->getDevice();

	shaderLayouts.gbuf.reflect(device, "shaders/gbuf.vert.spv", "shaders/gbuf.frag.spv");
	shaderLayouts.sponza.reflect(device, "shaders/sponza.vert.spv", "shaders/sponza.frag.spv");
	//shaderLayouts.composition.reflect(device, "shaders/composition.vert.spv", "shaders/composition.frag.spv");
	shaderLayouts.light_composition.reflect(device, "shaders/composition.vert.spv", "shaders/composelights.frag.spv");
	shaderLayouts.transparent.reflect(device, "shaders/transparent.vert.spv", "shaders/transparent.frag.spv");

	shaderLayouts.cluster.reflect(device, "shaders/cluster.comp.spv");
	shaderLayouts.lightAssignment.reflect(device, "shaders/lightassignment.comp.spv");
	pipelines.cluster.createPipeline(device, shaderLayouts.cluster);
	pipelines.lightAssignment.createPipeline(device, shaderLayouts.lightAssignment);

	shaderLayouts.offscreenShadow.reflect(device, "shaders/offscreen.vert.spv", "shaders/offscreen.frag.spv");





	//auto& subpassgbuf = renderPass->subpasses[MyRenderPass::kSubpass_GBUF];
	//const auto bind = AnimatedVertex::getBindingDescriptions();
	//const auto attr = AnimatedVertex::getAttributeDescriptions();
	//pipelines.character.shaderLayouts(shaderLayouts.gbuf)
	//	.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE), 0, 3)
	//	.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
	//	.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})
	//	.subpassIndex(MyRenderPass::kSubpass_GBUF)
	//	.createPipeline(renderPass->renderPass, vulkanBase->getDevice());

	const auto vbind = Vertex::getBindingDescriptions();
	const auto vattr = Vertex::getAttributeDescriptions();
	pipelines.opaque.shaderLayouts(shaderLayouts.sponza)
		.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE), 0, 3)
		.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {vbind.begin(), vbind.end()})
		.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {vattr.begin(), vattr.end()})
		.subpassIndex(MyRenderPass::kSubpass_GBUF)
		.createPipeline(deferredRenderPass->renderPass, vulkanBase->getDevice());
	
	//auto& subpasscomp = renderPass->subpasses[MyRenderPass::kSubpass_COMPOSE];
	pipelines.composition.shaderLayouts(shaderLayouts.light_composition)
		.cullMode(VK_CULL_MODE_NONE)
		.depthWriteEnable(VK_FALSE)
		.subpassIndex(MyRenderPass::kSubpass_COMPOSE)
		.createPipeline(deferredRenderPass->renderPass, vulkanBase->getDevice());

	pipelines.offscreen.shaderLayouts(shaderLayouts.offscreenShadow)
		.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE), 0, 1)
		.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {vbind.begin(), vbind.end()})
		.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {vattr.begin(), vattr.end()})
		.subpassIndex(0)
		.createPipeline(shadowRenderPass->renderPass, vulkanBase->getDevice());

	////auto& subpasstransparent = renderPass->subpasses[MyRenderPass::kSubpass_TRANSPARENT];
	//pipelines.transparentCharacter.shaderLayouts(shaderLayouts.transparent)
	//	.colorBlending(0)
	//	.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
	//	.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})
	//	.cullMode(VK_CULL_MODE_BACK_BIT)
	//	.depthWriteEnable(VK_FALSE)
	//	.subpassIndex(MyRenderPass::kSubpass_TRANSPARENT)
	//	.createPipeline(renderPass->renderPass, vulkanBase->getDevice());

}

//replace vulkanBase->createBuffer with this
SbAllocatedBuffer HelloTriangleApplication::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;

	SbAllocatedBuffer newBuffer;

	vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, nullptr);

	return newBuffer;
}

void HelloTriangleApplication::createDescriptorSets()
{
	//------------compute
	descriptorSets.cluster = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.cluster, 0);
	descriptorSets.cluster->addBufferBinding(0, shaderStorage.clusterSSBO);
	descriptorSets.cluster->addBufferBinding(1, shaderStorage.screenToViewUniform);
	descriptorSets.cluster->allocate(*descriptorPool.get());
	descriptorSets.cluster->updateDescriptors();

	descriptorSets.lights = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.lightAssignment, 1);
	descriptorSets.lights->addBufferBinding(0, shaderStorage.lightsSSBO);
	descriptorSets.lights->addBufferBinding(1, shaderStorage.lightIndexSSBO);
	descriptorSets.lights->addBufferBinding(2, shaderStorage.lightGridSSBO);
	descriptorSets.lights->allocate(*descriptorPool.get());
	descriptorSets.lights->updateDescriptors();

	descriptorSets.lightAssignment = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.lightAssignment, 0);
	descriptorSets.lightAssignment->addBufferBinding(0, shaderStorage.matrixUniform);
	descriptorSets.lightAssignment->addBufferBinding(1, shaderStorage.clusterSSBO);
	descriptorSets.lightAssignment->addBufferBinding(2, shaderStorage.screenToViewUniform);
	descriptorSets.lightAssignment->addBufferBinding(3, shaderStorage.lightIndexCountSSBO);
	descriptorSets.lightAssignment->allocate(*descriptorPool.get());
	descriptorSets.lightAssignment->updateDescriptors();

	//------------graphics

	descriptorSets.lights_g = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.light_composition, 1);
	descriptorSets.lights_g->addBufferBinding(0, shaderStorage.lightsSSBO);
	descriptorSets.lights_g->addBufferBinding(1, shaderStorage.lightIndexSSBO);
	descriptorSets.lights_g->addBufferBinding(2, shaderStorage.lightGridSSBO);
	descriptorSets.lights_g->addBufferBinding(3, shaderStorage.clusterSSBO);
	descriptorSets.lights_g->allocate(*descriptorPool.get());
	descriptorSets.lights_g->updateDescriptors();

	//descriptorSets.gbufDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.gbuf, 0);
	//descriptorSets.gbufDesc->addBufferBinding(0, shaderStorage.transformUniform);
	//gbufDesc->addBufferBinding(1, *shadingUniformBuffer);
	//descriptorSets.gbufDesc->allocate(*descriptorPool.get());
	//descriptorSets.gbufDesc->updateDescriptors();

	descriptorSets.matrixDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.sponza, 0);
	descriptorSets.matrixDesc->addBufferBinding(0, shaderStorage.matrixUniform);
	descriptorSets.matrixDesc->allocate(*descriptorPool.get());
	descriptorSets.matrixDesc->updateDescriptors();



	descriptorSets.compDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.light_composition, 0);
	descriptorSets.compDesc->addInputAttachmentBinding(0, MyRenderPass::kAttachment_POSITION);
	descriptorSets.compDesc->addInputAttachmentBinding(1, MyRenderPass::kAttachment_NORMAL);
	descriptorSets.compDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_ALBEDO);
	descriptorSets.compDesc->addBufferBinding(3, shaderStorage.screenToViewUniform);
	descriptorSets.compDesc->addBufferBinding(4, shaderStorage.cameraUniform);
	descriptorSets.compDesc->addBufferBinding(5, shaderStorage.matrixUniform);
	descriptorSets.compDesc->allocate(*descriptorPool.get());
	descriptorSets.compDesc->updateDescriptors();

	//descriptorSets.transDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.transparent, 0);
	//descriptorSets.transDesc->addBufferBinding(0, shaderStorage.transformUniform);
	//descriptorSets.transDesc->addBufferBinding(1, shaderStorage.shadingUniform);
	//descriptorSets.transDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_POSITION);
	//descriptorSets.transDesc->allocate(*descriptorPool.get());
	//descriptorSets.transDesc->updateDescriptors();

	auto& materials = sponza.scene.materials;
	for (size_t i = 0; i < materials.size(); i++)
	{
		if (!materials[i].hasAlpha &&
			materials[i].hasBump &&
			materials[i].hasSpecular &&
			i != 17)
		{

			materials[i].descriptor = new SbDescriptorSet(
				vulkanBase->getDevice(), *swapchain, shaderLayouts.sponza, 1);
			materials[i].descriptor->addImageBinding(0, textureSampler, &materials[i].diffuse->textureImageView);
			materials[i].descriptor->addImageBinding(1, textureSampler, &materials[i].bump->textureImageView);
			materials[i].descriptor->addImageBinding(2, textureSampler, &materials[i].specular->textureImageView);
			materials[i].descriptor->allocate(*descriptorPool.get());
			materials[i].descriptor->updateDescriptors();
		}
	}
}


void HelloTriangleApplication::createTextureSampler() {

	auto samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags(),
		vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		0, VK_TRUE, 16,
		VK_FALSE, vk::CompareOp::eAlways,
		0, 1,
		vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

	textureSampler = vulkanBase->getDevice().createSampler(samplerInfo);
}

//TODO make function take model argument?
void HelloTriangleApplication::createVertexBuffer() {

	using BufferUsage = vk::BufferUsageFlagBits;
	using MemoryProperty = vk::MemoryPropertyFlagBits;

	for (size_t i = 0; i < mymodel->meshes.size(); i++)
	{
		vk::DeviceSize bufferSize = sizeof(AnimatedVertex) * mymodel->meshes[i].vertexBuffer.size();
		vk::DeviceSize indexBufferSize = sizeof(unsigned int) * mymodel->meshes[i].indexBuffer.size();

		//------
		SbBuffer stagingBuffer = SbBuffer(*vulkanBase, bufferSize, BufferUsage::eTransferSrc,
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

void HelloTriangleApplication::createUniformBuffers() {
	constexpr glm::uvec3 clusterSizes = { 16, 12, 24 };
	constexpr uint32_t clusterCount = clusterSizes[0] * clusterSizes[1] * clusterSizes[2];

	shaderStorage.clusterSSBO = new SbUniformBuffer<AABB>(*vulkanBase, 1, clusterCount, 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	shaderStorage.screenToViewUniform = new SbUniformBuffer<ScreenToViewUniform>(*vulkanBase, 1);
	
	*(shaderStorage.screenToViewUniform->data()) = { 
		glm::inverse(cam.getProjectionMatrix()),
		glm::uvec4(clusterSizes[0], clusterSizes[1], clusterSizes[2], WIDTH / clusterSizes[0]),
		glm::uvec2(WIDTH, HEIGHT),
		(float)clusterSizes[2] / std::log2f(cam.zFar / cam.zNear), //scaling
		-((float)clusterSizes[2] * std::log2f(cam.zNear) / std::log2f(cam.zFar / cam.zNear)) //bias
	};
	shaderStorage.screenToViewUniform->writeToMappedMemory(0);

	shaderStorage.cameraUniform = new SbUniformBuffer<glm::vec4>(*vulkanBase, swapchain->getSize());	


	int num_lights = 1;
	shaderStorage.lightsSSBO = new SbUniformBuffer<PointLight>(*vulkanBase, 1, num_lights, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);	
	//TODO generate lights
	
	for (unsigned int i = 0; i < num_lights; ++i) 
	{
		PointLight* pLight = shaderStorage.lightsSSBO->data(i);
		//Fetching the light from the current scene
		pLight->position = glm::vec4(1.0, 1.0, 0 , 1);
		pLight->color = glm::vec4(1.0f);
		pLight->enabled = 1;
		pLight->intensity = 50.0f;
		pLight->range = 2.8f+(0.1f * i);
	}
	shaderStorage.lightsSSBO->writeToMappedMemory(0);


	//this is ridiculous, space for 100 light affecting every cluster at the same time...
	shaderStorage.lightIndexSSBO = new SbUniformBuffer<uint32_t>(*vulkanBase, swapchain->getSize(), clusterCount * 100, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	shaderStorage.lightGridSSBO = new SbUniformBuffer<LightGrid>(*vulkanBase, swapchain->getSize(), clusterCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	shaderStorage.lightIndexCountSSBO = new SbUniformBuffer<uint32_t>(*vulkanBase, swapchain->getSize(), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	
	shaderStorage.matrixUniform = new SbUniformBuffer<MatrixBufferObject>(*vulkanBase, swapchain->getSize());
	//shaderStorage.transformUniform = new SbUniformBuffer<UniformBufferObject>(*vulkanBase,swapchain->getSize());
	//shaderStorage.skeletonUniform = new SbUniformBuffer<glm::mat4>(*vulkanBase, swapchain->getSize(), 52 * 5); //room for 5 skeletons in buffer
	//shaderStorage.shadingUniform = new SbUniformBuffer<ShadingUBO>(*vulkanBase, 1, drawables.size());

	//for (size_t i = 0; i < drawables.size(); i++)
	//{
	//	ShadingUBO data = { drawables[i].AmbientColor, drawables[i].DiffuseColor, drawables[i].SpecularColor };
	//	shaderStorage.shadingUniform->writeBufferData(data, i);
	//}
	//shaderStorage.shadingUniform->writeToMappedMemory();
}

void HelloTriangleApplication::prepareCubeMap()
{

	//#define TEX_DIM 1024
	shadowCubeMap.width = 1024;
	shadowCubeMap.height = 1024;

	// 32 bit float format for higher precision
	VkFormat format = VK_FORMAT_R32_SFLOAT;

	// Cube map image description
	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { shadowCubeMap.width, shadowCubeMap.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 6; //this image is actually an array of 6 cube images!
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; //specifies that the image can be used to create a VkImageView of type VK_IMAGE_VIEW_TYPE_CUBE

	VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
		
	// Create cube map image	
	assert(VK_SUCCESS == vkCreateImage(vulkanBase->getDevice(),
		&imageCreateInfo, nullptr, &shadowCubeMap.image));

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vulkanBase->getDevice(), shadowCubeMap.image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = vulkanBase->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	assert(VK_SUCCESS == vkAllocateMemory(vulkanBase->getDevice(), &memAllocInfo, nullptr, &shadowCubeMap.deviceMemory));
	assert(VK_SUCCESS == vkBindImageMemory(vulkanBase->getDevice(), shadowCubeMap.image, shadowCubeMap.deviceMemory, 0));
	
	// Image barrier for optimal image (target)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 6;

	vulkanBase->commandPool->transitionImageLayout(shadowCubeMap.image,
 VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
		
	// Create sampler
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	assert(VK_SUCCESS == vkCreateSampler(vulkanBase->getDevice(), &sampler, nullptr, &shadowCubeMap.sampler));

	// Create image view
	VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.subresourceRange.layerCount = 6;
	view.image = shadowCubeMap.image;
	vkCreateImageView(vulkanBase->getDevice(), &view, nullptr, &shadowCubeMap.view);
}

void HelloTriangleApplication::createOffscreenRenderpass() 
{

}

// Prepare a new framebuffer for offscreen rendering
// The contents of this framebuffer are then
// copied to the different cube map faces
void HelloTriangleApplication::prepareOffscreenFramebuffer()
{
	//offscreenPass.width = 1024;
	//offscreenPass.height = 1024;

	VkFormat fbColorFormat = VK_FORMAT_R32_SFLOAT;

	VkDevice device = vulkanBase->getDevice();

	// Color attachment
	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = fbColorFormat;
	//imageCreateInfo.extent.width = offscreenPass.width;
	//imageCreateInfo.extent.height = offscreenPass.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// Image of the framebuffer is blit source
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();

	VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = fbColorFormat;
	colorImageView.flags = 0;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;

	VkMemoryRequirements memReqs;

	//assert(VK_SUCCESS == vkCreateImage(device, &imageCreateInfo, nullptr, &offscreenPass.color.image));
	//vkGetImageMemoryRequirements(device, offscreenPass.color.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanBase->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.color.mem);
	//vkBindImageMemory(device, offscreenPass.color.image, offscreenPass.color.mem, 0);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;

	//vulkanBase->commandPool->transitionImageLayout(offscreenPass.color.image, 
		//VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, subresourceRange);

	//colorImageView.image = offscreenPass.color.image;
	//assert(VK_SUCCESS == vkCreateImageView(device, &colorImageView, nullptr, &offscreenPass.color.view));

	// Depth stencil attachment
	imageCreateInfo.format = vulkanBase->findDepthFormat();
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = imageCreateInfo.format;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	//assert(VK_SUCCESS == vkCreateImage(device, &imageCreateInfo, nullptr, &offscreenPass.depth.image));
	//vkGetImageMemoryRequirements(device, offscreenPass.depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanBase->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//assert(VK_SUCCESS == vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.depth.mem));
	//assert(VK_SUCCESS == vkBindImageMemory(device, offscreenPass.depth.image, offscreenPass.depth.mem, 0));

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	//vulkanBase->commandPool->transitionImageLayout(offscreenPass.depth.image,
	//	VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, subresourceRange);

	//depthStencilView.image = offscreenPass.depth.image;
	//assert(VK_SUCCESS == vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPass.depth.view));

	VkImageView attachments[2];
	//attachments[0] = offscreenPass.color.view;
	//attachments[1] = offscreenPass.depth.view;

	//VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
	//fbufCreateInfo.renderPass = offscreenPass.renderPass;
	//fbufCreateInfo.attachmentCount = 2;
	//fbufCreateInfo.pAttachments = attachments;
	//fbufCreateInfo.width = offscreenPass.width;
	//fbufCreateInfo.height = offscreenPass.height;
	//fbufCreateInfo.layers = 1;

	//assert(VK_SUCCESS == vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offscreenPass.frameBuffer));
}

void HelloTriangleApplication::prepareOffscreenRenderpass()
{
	VkAttachmentDescription osAttachments[2] = {};

	// Find a suitable depth format	
	VkFormat validDepthFormat = vulkanBase->findDepthFormat();//vks::tools::getSupportedDepthFormat(physicalDevice, &fbDepthFormat);
	//assert(validDepthFormat);

	osAttachments[0].format = VK_FORMAT_R32_SFLOAT;
	osAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	osAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	osAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	osAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	osAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	osAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	osAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment
	osAttachments[1].format = validDepthFormat;
	osAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	osAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	osAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	osAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	osAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	osAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	osAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pDepthStencilAttachment = &depthReference;

	VkRenderPassCreateInfo renderPassCreateInfo = vks::initializers::renderPassCreateInfo();
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = osAttachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;

	//assert(VK_SUCCESS == vkCreateRenderPass(vulkanBase->getDevice(), &renderPassCreateInfo, nullptr, &offscreenPass.renderPass));
}

void HelloTriangleApplication::setupDescriptorSetLayout()
{
	VkDevice device = vulkanBase->getDevice();


	// Shared pipeline layout
	//std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0 : Vertex shader uniform buffer
	//	vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		// Binding 1 : Fragment shader image sampler (cube map)
	//	vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
	//};

	//VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
	//assert(VK_SUCCESS == 
		//vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout);

	// 3D scene pipeline layout
	//VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	//assert(VK_SUCCESS == vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.scene));

	// Offscreen pipeline layout
	// Push constants for cube map face view matrices
	//VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
	// Push constant ranges are part of the pipeline layout
	//pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	//pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	//assert(VK_SUCCESS == vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayouts.offscreen)=;
}

// Updates a single cube map face
// Renders the scene with face's view and does a copy from framebuffer to cube face
// Uses push constants for quick update of view matrix for the current cube map face
void HelloTriangleApplication::updateCubeFace(uint32_t faceIndex, VkCommandBuffer commandBuffer)
{
	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	// Reuse render pass from example pass
	//renderPassBeginInfo.renderPass = offscreenPass.renderPass;
	//renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer;
	//renderPassBeginInfo.renderArea.extent.width = offscreenPass.width;
	//renderPassBeginInfo.renderArea.extent.height = offscreenPass.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	// Update view matrix via push constant

	glm::mat4 viewMatrix = glm::mat4(1.0f);
	switch (faceIndex)
	{
	case 0: // POSITIVE_X
		viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case 1:	// NEGATIVE_X
		viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case 2:	// POSITIVE_Y
		viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case 3:	// NEGATIVE_Y
		viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case 4:	// POSITIVE_Z
		viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case 5:	// NEGATIVE_Z
		viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	}

	// Render scene from cube face's point of view
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Update shader push constant block
	// Contains current face view matrix
	//vkCmdPushConstants(
	//	commandBuffer,
	//	pipelineLayouts.offscreen,
	//	VK_SHADER_STAGE_VERTEX_BIT,
	//	0,
	//	sizeof(glm::mat4),
	//	&viewMatrix);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.offscreen.handle);//todo create pipeline
	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.offscreen, 0, 1, &descriptorSets.offscreen, 0, NULL);
	//models.scene.draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
	// Make sure color writes to the framebuffer are finished before using it as transfer source
	//vks::tools::setImageLayout(
	//	commandBuffer,
	//	offscreenPass.color.image,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//vks::helper::transitionImageLayout(commandBuffer, offscreenPass.color.image,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);


	VkImageSubresourceRange cubeFaceSubresourceRange = {};
	cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cubeFaceSubresourceRange.baseMipLevel = 0;
	cubeFaceSubresourceRange.levelCount = 1;
	cubeFaceSubresourceRange.baseArrayLayer = faceIndex;
	cubeFaceSubresourceRange.layerCount = 1;

	// Change image layout of one cubemap face to transfer destination
	//vks::tools::setImageLayout(
	//	commandBuffer,
	//	shadowCubeMap.image,
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	cubeFaceSubresourceRange);
	vks::helper::transitionImageLayout(commandBuffer, shadowCubeMap.image, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cubeFaceSubresourceRange);

	// Copy region for transfer from framebuffer to cube face
	VkImageCopy copyRegion = {};

	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = { 0, 0, 0 };

	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.baseArrayLayer = faceIndex;
	copyRegion.dstSubresource.mipLevel = 0;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset = { 0, 0, 0 };

	copyRegion.extent.width = shadowCubeMap.width;
	copyRegion.extent.height = shadowCubeMap.height;
	copyRegion.extent.depth = 1;

	// Put image copy into command buffer
	//vkCmdCopyImage(
	//	commandBuffer,
	//	offscreenPass.color.image,
	//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//	shadowCubeMap.image,
	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	1,
	//	&copyRegion);

	// Transform framebuffer color attachment back
	//vks::tools::setImageLayout(
	//	commandBuffer,
	//	offscreenPass.color.image,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//vks::helper::transitionImageLayout(commandBuffer,
	//	offscreenPass.color.image,
	//	VK_IMAGE_ASPECT_COLOR_BIT,
	//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	// Change image layout of copied face to shader read
	//vks::tools::setImageLayout(
	//	commandBuffer,
	//	shadowCubeMap.image,
	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//	cubeFaceSubresourceRange);
	vks::helper::transitionImageLayout(commandBuffer, 
		shadowCubeMap.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		cubeFaceSubresourceRange);

}

void HelloTriangleApplication::createDescriptorPool() {

	descriptorPool = std::make_unique<SbDescriptorPool>(vulkanBase->getDevice());

	std::vector<VkDescriptorPoolSize> poolSizes(4);
	poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(100) };//(swapchain->getSize() * 2) }; 
	poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(100) };//(swapchain->getSize() * 2) };
	poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(100) };//(swapchain->getSize() * 2) };
	poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(100) };//(swapchain->getSize() * 4) };

	descriptorPool->createDescriptorPool(poolSizes, static_cast<uint32_t>(swapchain->getSize()) * 100);

}

void HelloTriangleApplication::createCommandBuffers() {
	
	computeCommandBuffer = vulkanBase->commandPool->beginSingleTimeCommands();
	 
	vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.cluster.handle);
	VkDescriptorSet set[] = { descriptorSets.cluster->allocatedDSs[0]};
	vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.cluster.getLayout(), 0, 1, set, 0, nullptr);

	//constexpr uint32_t workGroupSize = 256;
	//constexpr uint32_t groupcount = ((clusterCount) / workGroupSize) + 1;
	auto viewUniform = shaderStorage.screenToViewUniform->data();
	vkCmdDispatch(computeCommandBuffer, viewUniform->tileSizes.x, viewUniform->tileSizes.y, viewUniform->tileSizes.z);

	vulkanBase->commandPool->endSingleTimeCommands(computeCommandBuffer);
	
	shaderStorage.clusterSSBO->readFromMappedMemory(0);

	//----------------

	commandBuffers.resize(swapchain->getSize());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vulkanBase->commandPool->handle;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(vulkanBase->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t cmdIdx = 0; cmdIdx < commandBuffers.size(); cmdIdx++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(commandBuffers[cmdIdx], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//compute light assignments
		vkCmdBindPipeline(commandBuffers[cmdIdx],
			VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.lightAssignment.handle);		
		VkDescriptorSet sets[] = { 
			descriptorSets.lightAssignment->allocatedDSs[cmdIdx], 
			descriptorSets.lights->allocatedDSs[cmdIdx] 
		};
		vkCmdBindDescriptorSets(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_COMPUTE,
			pipelines.lightAssignment.getLayout(), 0, 2, sets, 0, nullptr);
		vkCmdDispatch(commandBuffers[cmdIdx], 1, 1, 6); //todo hard coded

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = deferredRenderPass->renderPass;
		renderPassInfo.framebuffer = deferredFrameBuffers[cmdIdx].frameBuffer;
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

		vkCmdBeginRenderPass(commandBuffers[cmdIdx], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport(
			(float)swapchain->swapchainCI.imageExtent.width,
			(float)swapchain->swapchainCI.imageExtent.height, 0.0f, 1.0f);
		vkCmdSetViewport(commandBuffers[cmdIdx], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(swapchain->swapchainCI.imageExtent, 0, 0);
		vkCmdSetScissor(commandBuffers[cmdIdx], 0, 1, &scissor);

		{
			//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.character.handle);
			//
			//for (size_t j = 0; j < drawables.size() - 1; j++)
			//{
			//	uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
			//	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.character.getLayout(),
			//		0, 1, &gbufDesc->allocatedDSs[i], 0, &offset); //todo dynamic offset count should be what?
			//
			//	VkBuffer vert[] = { drawables[j].VertexBuffer };
			//	VkDeviceSize offsets[] = { 0 };
			//	vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);
			//
			//	//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
			//	vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			//
			//	vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
			//}

			vkCmdBindPipeline(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.opaque.handle);

			VkDeviceSize offsets[] = { 0 };
			VkBuffer vert[] = { sponza.scene.vertexBuffer.buffer };
			vkCmdBindVertexBuffers(commandBuffers[cmdIdx], 0, 1, vert, offsets);
			vkCmdBindIndexBuffer(commandBuffers[cmdIdx], sponza.scene.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.opaque.getLayout(), 0, 1, sets, 0, nullptr);
			for (size_t meshIdx = 0; meshIdx < sponza.scene.meshes.size(); meshIdx++)
			{
				auto currentMaterial = sponza.scene.meshes[meshIdx].material;
				if (!currentMaterial->hasAlpha &&
					currentMaterial->hasBump &&
					currentMaterial->hasSpecular &&
					cmdIdx != 17)
				{
					VkDescriptorSet sets[] = { descriptorSets.matrixDesc->allocatedDSs[0], currentMaterial->descriptor->allocatedDSs[0] };
					vkCmdBindDescriptorSets(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.opaque.getLayout(), 0, 2, sets, 0, nullptr);

					vkCmdDrawIndexed(commandBuffers[cmdIdx], sponza.scene.meshes[meshIdx].indexCount, 1, sponza.scene.meshes[meshIdx].indexBase, 0, 0);
				}
			}
		}


		vkCmdNextSubpass(commandBuffers[cmdIdx], VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.composition.handle);
			std::vector<VkDescriptorSet> sets = { 
				descriptorSets.compDesc->allocatedDSs[cmdIdx],
				descriptorSets.lights_g->allocatedDSs[cmdIdx]
			};
			vkCmdBindDescriptorSets(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.composition.getLayout(),
				0, sets.size(), sets.data(), 0, NULL);
			vkCmdDraw(commandBuffers[cmdIdx], 3, 1, 0, 0);
		}


		vkCmdNextSubpass(commandBuffers[cmdIdx], VK_SUBPASS_CONTENTS_INLINE);
		{

			//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.transparentCharacter.handle);
			//
			//for (size_t j = drawables.size() - 1; j < drawables.size(); j++)
			//{
			//	uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
			//	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.transparentCharacter.getLayout(),
			//		0, 1, &transDesc->allocatedDSs[i], 0, &offset);
			//
			//	VkBuffer vert[] = { drawables[j].VertexBuffer };
			//	VkDeviceSize offsets[] = { 0 };
			//	vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);
			//
			//	//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
			//	vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			//
			//	vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
			//}
		}

		vkCmdEndRenderPass(commandBuffers[cmdIdx]);

		if (vkEndCommandBuffer(commandBuffers[cmdIdx]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage) {

	auto sideways = glm::normalize(glm::cross(cam.globalUp, cam.forward));
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), -(float)delta_mouse_pos.x * 0.001f, cam.globalUp);
	rot = glm::rotate(rot, (float)delta_mouse_pos.y * 0.001f, sideways);
	//glm::quat horizontal = glm::rotate(glm::quat(), , cam.globalUp);
	//glm::quat vertical = glm::rotate(glm::quat(), (float)delta_mouse_pos.x, sideways);

	auto deltaMouseQuat = glm::quat(glm::vec3(delta_mouse_pos.y, delta_mouse_pos.x, 0)*0.01f);
	cam.forward = rot * glm::vec4(cam.forward, 1.0f);

	(*shaderStorage.cameraUniform->data()) = { cam.position.x, cam.position.y, cam.position.z, 1 };
	shaderStorage.cameraUniform->writeToMappedMemory(currentImage);


	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	//float TicksPerSecond = modelScene->mAnimations[0]->mTicksPerSecond != 0 ?
	//	modelScene->mAnimations[0]->mTicksPerSecond : 25.0f;
	//float TimeInTicks = time * TicksPerSecond;
	//float AnimationTime = fmod(TimeInTicks, modelScene->mAnimations[0]->mDuration);

	//float s = 0.5f * std::sinf(time / (3.14f)) + 0.5f;
	//mymodel->skeletalAnimationComponent.evaluate(deltaTime, std::vector<float> { s });

	//UniformBufferObject ubo = {};
	//ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f))
	//	* glm::scale(glm::mat4(1.0f), { 0.02f,0.02f ,0.02f });
	//ubo.view = cam.getViewMatrix();
	//ubo.proj = cam.getProjectionMatrix();
	//memcpy(ubo.boneTransforms, mymodel->skeletalAnimationComponent.transformations.data(),
	//	sizeof(glm::mat4) * mymodel->skeletalAnimationComponent.transformations.size());
	//shaderStorage.transformUniform->writeBufferData(ubo);
	//shaderStorage.transformUniform->writeToMappedMemory(currentImage);



	auto buf = shaderStorage.matrixUniform->data();
	buf->model = //glm::rotate(glm::mat4(1.0f), time * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f))
			 glm::scale(glm::mat4(1.0f), { 0.05f,0.05f ,0.05f });
	buf->view = cam.getViewMatrix();
	buf->proj = cam.getProjectionMatrix();
	//matrixUniformBuffer->writeBufferData(buf);
	shaderStorage.matrixUniform->writeToMappedMemory(currentImage);


}

void HelloTriangleApplication::drawFrame() {
	uint32_t imageIndex = swapchain->acquireNextImage();
	updateUniformBuffer(imageIndex);
	std::vector<VkCommandBuffer> cmds{ commandBuffers[imageIndex] };
	std::vector<VkSemaphore> waitSem{ swapchain->getImageAvailableSemaphore() };
	std::vector<VkSemaphore> signalSem{ swapchain->getRenderFinishedSemaphore() };
	vulkanBase->submitCommandBuffers(cmds, waitSem, signalSem, swapchain->getInFlightFence());
	swapchain->presentImage(imageIndex, signalSem);
	swapchain->updateFrameInFlightCounter();
	
}

std::vector<char> HelloTriangleApplication::readFile(const std::string& filename) {
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
