#include "HelloTriangleApplication.h"

#include "VulkanInitializers.hpp"

//#include "SbDescriptorSet.h"

#include "ResourceManager.h"

#include "Sponza.h"
#include "AnimatedVertex.h"              // for AnimatedVertex
#include "array"                         // for array
#include "chrono"                        // for high_resolution_clock, steady_clock::time_point
#include "cmath"                         // for sinf
#include "corecrt_malloc.h"              // for _aligned_free, _aligned_malloc
#include "corecrt_math.h"                // for sinf
#include "GLFW/glfw3.h"                  // for glfwCreateWindow, glfwDestroyWindow, glfwGetFramebufferSize, glfwGetWindowUserPointer, glfwInit, glfwPollEvents, glfwSetFramebufferSizeCallback, glfwSetKeyCallback, glfwSetWindowUserPointer, glfwTerminate, glfwWaitEvents, glfwWindowHint, glfwWindowShouldClose, GLFWwindow, GLFW_CLIENT_API, GLFW_NO_API
#include "iosfwd"                        // for ifstream
#include "Model.h"                       // for AnimatedModel, AnimatedMesh, Material
#include "SbBuffer.h"                    // for SbBuffer
#include "SbVulkanBase.h"                // for SbVulkanBase, enableValidationLayers
#include "SkeletalAnimationComponent.h"  // for SkeletalAnimationComponent
#include "stddef.h"                      // for offsetof
#include "stdexcept"                     // for runtime_error
#include "vcruntime_new.h"               // for operator new, operator delete
#include "vcruntime_string.h"            // for memcpy
#include "Vertex.h"                      // for Vertex


#include "SbSwapchain.h"
#include "MyRenderPass.h"
#include "SbDescriptorSet.h"
#include "SbDescriptorPool.h"
#include "SbCommandPool.h"


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
	if (shadingUboData)
	{
		alignedFree(shadingUboData);
	}
	delete(mymodel);
}

void HelloTriangleApplication::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void HelloTriangleApplication::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//if (key == GLFW_KEY_E && action == GLFW_PRESS);
	//activate_airship();
	std::cout << "basic key callback" << std::endl;
}

void HelloTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void HelloTriangleApplication::initVulkan() {
	createInstance();
	vulkanBase->commandPool = std::unique_ptr<SbCommandPool>(new SbCommandPool(*vulkanBase));

	ResourceManager::getInstance().vkBase = vulkanBase; //todo ugly bad

	createTextureSampler();
	sponza = new Sponza();
	sponza->load(*vulkanBase);


	swapchain = new SbSwapchain(*vulkanBase);
	swapchain->createSwapChain(vulkanBase->surface, window);

	renderPass = new MyRenderPass(*vulkanBase, *swapchain);
	 

	swapchain->createFramebuffersForRenderpass(renderPass->renderPass);
	
	mymodel = new AnimatedModel("jump.fbx");
	createVertexBuffer();

	createUniformBuffers();
	createDescriptorPool();

	createPipelines();
	createDescriptorSets();

	createCommandBuffers();
	swapchain->createSyncObjects(MAX_FRAMES_IN_FLIGHT);
}

void HelloTriangleApplication::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - frameTime).count();
		frameTime = std::chrono::high_resolution_clock::now();
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(vulkanBase->logicalDevice->device);
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

void HelloTriangleApplication::cleanup() {
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

void HelloTriangleApplication::recreateSwapChain() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(vulkanBase->logicalDevice->device);

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
	vulkanBase = new SbVulkanBase();
	vulkanBase->createInstance(enableValidationLayers);
	vulkanBase->setupDebugMessenger(enableValidationLayers);//moved here from setupdebugmessenger
	vulkanBase->createSurface(window); //moved from createsurface

	vulkanBase->pickPhysicalDevice();
	vulkanBase->createLogicalDevice();
}


void HelloTriangleApplication::createPipelines() {
	auto& subpassgbuf = renderPass->subpasses[MyRenderPass::kSubpass_GBUF];
	const auto& bind = AnimatedVertex::getBindingDescriptions();
	const auto& attr = AnimatedVertex::getAttributeDescriptions();
	SbPipeline newPipeline = SbPipeline();
	newPipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/gbuf.vert.spv", "shaders/gbuf.frag.spv")
		.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(vk::ColorComponentFlags(), false), 0, 3)
		.vertexBindingDescription(std::vector<vk::VertexInputBindingDescription> {bind.begin(), bind.end()})
		.vertexAttributeDescription(std::vector<vk::VertexInputAttributeDescription> {attr.begin(), attr.end()})
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device, MyRenderPass::kSubpass_GBUF); //todo this enum should be automatic
	subpassgbuf.pipelines.push_back({
		newPipeline.handle, newPipeline.shaderLayout.bindings, newPipeline.shaderLayout.DSL
	});

	const std::array<VkVertexInputBindingDescription, 1>& vertexBind = Vertex::getBindingDescriptions();
	const std::array<VkVertexInputAttributeDescription, 4>& vertexAttr = Vertex::getAttributeDescriptions();

	struct SpecializationData {
		int32_t discard = 0;
	} specializationData;
	auto specEntry = vks::initializers::specializationMapEntry(0, offsetof(SpecializationData, discard), sizeof(int32_t));
	vk::SpecializationInfo specializationInfo = vks::initializers::specializationInfo(1, &specEntry, sizeof(specializationData), &specializationData);

	//SbRenderpass::Subpass subpassScene = renderPass->subpasses[MyRenderPass::kSubpass_SCENE];
	newPipeline = SbPipeline();
	newPipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/sponza.vert.spv", "shaders/sponza.frag.spv")
		.shaderStageSpecialization(1, &specializationInfo)
		.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(vk::ColorComponentFlags(), false), 0, 3)
		.vertexBindingDescription(std::vector<vk::VertexInputBindingDescription> {vertexBind.begin(), vertexBind.end()})
		.vertexAttributeDescription(std::vector<vk::VertexInputAttributeDescription> {vertexAttr.begin(), vertexAttr.end()})
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device, MyRenderPass::kSubpass_GBUF);
	subpassgbuf.pipelines.push_back({ 
		newPipeline.handle, newPipeline.shaderLayout.bindings, newPipeline.shaderLayout.DSL 
	});

	//skip transparent for now
	//specializationData.discard = 1;
	//subpassgbuf.pipelines.push_back(subpassgbuf.pipelines.back());
	//subpassgbuf.pipelines.back().shaderStageSpecialization(1, &specializationInfo)
	//	.cullMode(VK_CULL_MODE_NONE)
	//	.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device, MyRenderPass::kSubpass_GBUF);

	//compose
	auto& subpasscomp = renderPass->subpasses[MyRenderPass::kSubpass_COMPOSE];
	newPipeline = SbPipeline();
	newPipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/composition.vert.spv", "shaders/composition.frag.spv")
		.cullMode(vk::CullModeFlagBits::eNone)
		.depthWriteEnable(false)
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device, MyRenderPass::kSubpass_COMPOSE);
	subpasscomp.pipelines.push_back({
		newPipeline.handle, newPipeline.shaderLayout.bindings, newPipeline.shaderLayout.DSL
	});

	//Transparent pass
	auto& subpasstransparent = renderPass->subpasses[MyRenderPass::kSubpass_TRANSPARENT];
	newPipeline = SbPipeline();
	newPipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/transparent.vert.spv", "shaders/transparent.frag.spv")
		.colorBlending(0)
		.vertexBindingDescription(std::vector<vk::VertexInputBindingDescription> {bind.begin(), bind.end()})
		.vertexAttributeDescription(std::vector<vk::VertexInputAttributeDescription> {attr.begin(), attr.end()})
		.cullMode(vk::CullModeFlagBits::eBack)
		.depthWriteEnable(false)
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device, MyRenderPass::kSubpass_TRANSPARENT);
	subpasstransparent.pipelines.push_back({
		newPipeline.handle, newPipeline.shaderLayout.bindings, newPipeline.shaderLayout.DSL
	});

}

void HelloTriangleApplication::createDescriptorSets()
{
	auto gbufPipeline0 = renderPass->subpasses[MyRenderPass::kSubpass_GBUF].pipelines[0];
	gbufDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, 
		gbufPipeline0.DSL[0], gbufPipeline0.bindings[0]);
	gbufDesc->addBufferBinding(0, *transformUniformBuffer);
	gbufDesc->addBufferBinding(1, *shadingUniformBuffer);
	gbufDesc->allocate(*descriptorPool);
	gbufDesc->updateDescriptors();

	auto composePipeline0 = renderPass->subpasses[MyRenderPass::kSubpass_COMPOSE].pipelines[0];
	compDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, 
		composePipeline0.DSL[0], composePipeline0.bindings[0]);
	compDesc->addInputAttachmentBinding(0, MyRenderPass::kAttachment_POSITION);
	compDesc->addInputAttachmentBinding(1, MyRenderPass::kAttachment_NORMAL);
	compDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_ALBEDO);
	compDesc->allocate(*descriptorPool);
	compDesc->updateDescriptors();

	auto transparentPipeline0 = renderPass->subpasses[MyRenderPass::kSubpass_TRANSPARENT].pipelines[0];
	transDesc = new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, 
		transparentPipeline0.DSL[0], transparentPipeline0.bindings[0]);
	transDesc->addBufferBinding(0, *transformUniformBuffer);
	transDesc->addBufferBinding(1, *shadingUniformBuffer);
	transDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_POSITION);
	transDesc->allocate(*descriptorPool);
	transDesc->updateDescriptors();

	//set 0 vp matrix
	auto gbufPipeline1 = renderPass->subpasses[MyRenderPass::kSubpass_GBUF].pipelines[1];
	sceneGlobalDesc = new SbDescriptorSet(vulkanBase->getDevice(), 
		*swapchain, gbufPipeline1.DSL[0], gbufPipeline1.bindings[0]);
	sceneGlobalDesc->addBufferBinding(0, *vpTransformBuffer);
	sceneGlobalDesc->allocate(*descriptorPool);
	sceneGlobalDesc->updateDescriptors();

	//set 1	
	sponza->scene.prepareMaterialDescriptors(*vulkanBase, *descriptorPool, gbufPipeline1.DSL[1]);
	
}


void HelloTriangleApplication::createTextureSampler()
{
	auto samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags(),
		vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		0, VK_TRUE, 16,
		VK_FALSE, vk::CompareOp::eAlways,
		0, 1, //todo do we need many many samplers to allow for more lod/mips?
		vk::BorderColor::eIntOpaqueBlack, VK_FALSE);
	
	//materialSampler = vulkanBase->getDevice().createSampler(samplerInfo);
	ResourceManager::getInstance().createTextureSampler("shared sampler", samplerInfo);
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
	transformUniformBuffer = new SbUniformBuffer<UniformBufferObject>(*vulkanBase,	swapchain->getSize());

	vpTransformBuffer = new SbUniformBuffer<VPTransformBuffer>(
		*vulkanBase, swapchain->getSize());
	VPTransformBuffer VPUBO = {};
	VPUBO.view = cam.getViewMatrix();
	VPUBO.proj = cam.getProjectionMatrix();	
	vpTransformBuffer->writeBufferData(VPUBO);
	vpTransformBuffer->copyDataToBufferMemory(*vulkanBase);

	shadingUniformBuffer =  new SbUniformBuffer<ShadingUBO>(*vulkanBase, 1, drawables.size());
	for (size_t i = 0; i < drawables.size(); i++)
	{
		ShadingUBO data = { drawables[i].AmbientColor, drawables[i].DiffuseColor, drawables[i].SpecularColor };
		shadingUniformBuffer->writeBufferData(data, i);
	}
	shadingUniformBuffer->copyDataToBufferMemory(*vulkanBase);
}

void HelloTriangleApplication::createDescriptorPool() {

	descriptorPool = new SbDescriptorPool(vulkanBase->logicalDevice->device);

	std::vector<VkDescriptorPoolSize> poolSizes(4);
	poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(1000) };//(swapchain->getSize() * 2) }; 
	poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(1000) };//(swapchain->getSize() * 2) };
	poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(1000) };//(swapchain->getSize() * 2) };
	poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(1000) };//(swapchain->getSize() * 4) };

	descriptorPool->createDescriptorPool(poolSizes, static_cast<uint32_t>(swapchain->getSize()) * 100); //TODO proper pool handling (See VKguide)

}

void HelloTriangleApplication::createCommandBuffers() {
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

		//character pipeline
		{
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(MyRenderPass::kSubpass_GBUF, 0));

			for (size_t j = 0; j < drawables.size() - 1; j++)
			{
				uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipelineLayout(MyRenderPass::kSubpass_GBUF),
					0, 1, &gbufDesc->allocatedDSs[i], 0, &offset); //todo dynamic offset count should be what?

				VkBuffer vert[] = { drawables[j].VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vert, offsets);

				//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer[j], 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindIndexBuffer(commandBuffers[i], drawables[j].IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(commandBuffers[i], drawables[j].IndexCount, 1, 0, 0, 0);
			}
		}			
		//scene pipelines
		{
			auto& sceneRef = sponza->scene;
			////opaque
			VkPipeline pipeline = renderPass->getSubpassPipeline(MyRenderPass::kSubpass_GBUF, 1); //todo these pipelines should have enums
			VkPipelineLayout pipelineLayout = renderPass->getSubpassPipelineLayout(MyRenderPass::kSubpass_GBUF, 1);			
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline); 
			//
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout, 0, 1, &sceneGlobalDesc->allocatedDSs[i], 0, NULL);
			//
			//// Render from global buffer using index offsets
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &sceneRef.vertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], sceneRef.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			//
			for (auto mesh : sceneRef.meshes)
			{	
				if (sceneRef.materials.hasAlpha[mesh.material])
				{
					continue;
				}
				auto& materialDescriptorSet = sceneRef.materials.descriptorSets[mesh.material];
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
					pipelineLayout, 1, 1, &materialDescriptorSet, 0, NULL);
				vkCmdDrawIndexed(commandBuffers[i], mesh.indexCount, 1, 0, mesh.indexBase, 0);
			}
		
			//transparent skipped
			//pipeline = renderPass->getSubpassPipeline(MyRenderPass::kSubpass_GBUF, 2); //todo these pipelines should have enums
			//pipelineLayout = renderPass->getSubpassPipelineLayout(MyRenderPass::kSubpass_GBUF, 2);
			//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			//	renderPass->getSubpassPipeline(MyRenderPass::kSubpass_GBUF, 2));
			//for (auto mesh : sceneRef.meshes)
			//{
			//	if (sceneRef.materials.hasAlpha[mesh.material])
			//	{
			//		auto& materialDescriptorSet = sceneRef.materials.descriptorSets[mesh.material];
			//		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
			//			pipelineLayout, 0, 1, &materialDescriptorSet, 0, NULL);
			//		vkCmdDrawIndexed(commandBuffers[i], mesh.indexCount, 1, 0, mesh.indexBase, 0);
			//	}
			//}
		}

		vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(MyRenderPass::kSubpass_COMPOSE));
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipelineLayout(MyRenderPass::kSubpass_COMPOSE), 
				0, 1, &compDesc->allocatedDSs[i], 0, NULL);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		}


		vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
		{

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(MyRenderPass::kSubpass_TRANSPARENT));

			for (size_t j = drawables.size() - 1; j < drawables.size(); j++)
			{
				uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipelineLayout(MyRenderPass::kSubpass_TRANSPARENT),
					0, 1, &transDesc->allocatedDSs[i], 0, &offset);

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

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage) {

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	
	float s = 0.5f * std::sinf(time / (3.14f)) + 0.5f;
	mymodel->skeletalAnimationComponent.evaluate(deltaTime, std::vector<float> { s });

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), { 0.01f,0.01f ,0.01f });
	ubo.view = cam.getViewMatrix();
	ubo.proj = cam.getProjectionMatrix();
	memcpy(ubo.boneTransforms, mymodel->skeletalAnimationComponent.transformations.data(),
		sizeof(glm::mat4) * mymodel->skeletalAnimationComponent.transformations.size());
	transformUniformBuffer->writeBufferData(ubo);
	transformUniformBuffer->copyDataToBufferMemory(*vulkanBase, currentImage);

}

void HelloTriangleApplication::drawFrame() {
	uint32_t imageIndex = swapchain->acquireNextImage();
	updateUniformBuffer(imageIndex);
	std::vector<vk::CommandBuffer> cmds{ commandBuffers[imageIndex] };
	std::vector<vk::Semaphore> waitSem{ swapchain->getImageAvailableSemaphore() };
	std::vector<vk::Semaphore> signalSem{ swapchain->getRenderFinishedSemaphore() };
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
