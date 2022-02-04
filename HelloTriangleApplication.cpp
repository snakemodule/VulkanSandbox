#include "HelloTriangleApplication.h"

#include "VulkanHelperFunctions.hpp"
#include "VulkanInitializers.hpp"

#include "SbDescriptorPool.h"
//#include "SbDescriptorSet.h"
#include "MyRenderPass.h"
#include "SbTextureImage.h"

#include "ResourceManager.h"

#include "Sponza.h"

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

void HelloTriangleApplication::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

std::vector<int> keyPresses;

void HelloTriangleApplication::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{		
			keyPresses.push_back(key);		
	}
	
	
	std::cout << "basic key callback" << std::endl;
}

void HelloTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void HelloTriangleApplication::initVulkan() {
	createInstance();
	vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);

	ResourceManager::getInstance().vkBase = vulkanBase.get(); //todo ugly bad



	sponza.load(vulkanBase.get());



	swapchain = std::make_unique<SbSwapchain>(*vulkanBase);
	swapchain->createSwapChain(vulkanBase->surface, window);

	MyRenderPass* pass = new MyRenderPass(*vulkanBase, *swapchain);
	renderPass = std::unique_ptr<MyRenderPass>(pass);

	swapchain->createFramebuffersForRenderpass(renderPass->renderPass);

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

void HelloTriangleApplication::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - frameTime).count();
		frameTime = std::chrono::high_resolution_clock::now();
		glfwPollEvents();
		handleKeyPresses();
		drawFrame();
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

	for (auto framebuffer : swapchain->swapChainFramebuffers) {
		vkDestroyFramebuffer(vulkanBase->getDevice(), framebuffer, nullptr);
	}

	vkFreeCommandBuffers(vulkanBase->getDevice(), vulkanBase->commandPool->handle, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	//vkDestroyPipeline(vulkanBase->getDevice(), attachmentWriteSubpass.Pipeline, nullptr);
	//vkDestroyPipeline(vulkanBase->getDevice(), attachmentReadSubpass.Pipeline, nullptr);
	//vkDestroyPipelineLayout(vulkanBase->getDevice(), attachmentWriteSubpass.descriptorSets.get()->pipelineLayout, nullptr);
	//vkDestroyPipelineLayout(vulkanBase->getDevice(), attachmentReadSubpass.descriptorSets.get()->pipelineLayout, nullptr);
	vkDestroyRenderPass(vulkanBase->getDevice(), renderPass->renderPass, nullptr);

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

	pipelines.composition.specializeFrag(&specialization).createPipeline(renderPass->renderPass, vulkanBase->getDevice());

	//rebuild command buffers
	createCommandBuffers();
}

void HelloTriangleApplication::createPipelines()
{
	shaderLayouts.gbuf.reflect(vulkanBase->getDevice(), "shaders/gbuf.vert.spv", "shaders/gbuf.frag.spv");
	shaderLayouts.sponza.reflect(vulkanBase->getDevice(), "shaders/sponza.vert.spv", "shaders/sponza.frag.spv");
	shaderLayouts.composition.reflect(vulkanBase->getDevice(), "shaders/composition.vert.spv", "shaders/composition.frag.spv");
	shaderLayouts.transparent.reflect(vulkanBase->getDevice(), "shaders/transparent.vert.spv", "shaders/transparent.frag.spv");

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
		.createPipeline(renderPass->renderPass, vulkanBase->getDevice());

	//auto& subpasscomp = renderPass->subpasses[MyRenderPass::kSubpass_COMPOSE];
	pipelines.composition.shaderLayouts(shaderLayouts.composition)
		.cullMode(VK_CULL_MODE_NONE)
		.depthWriteEnable(VK_FALSE)
		.subpassIndex(MyRenderPass::kSubpass_COMPOSE)
		.createPipeline(renderPass->renderPass, vulkanBase->getDevice());

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

void HelloTriangleApplication::createDescriptorSets()
{
	gbufDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.gbuf, 0));
	gbufDesc->addBufferBinding(0, *transformUniformBuffer);
	//gbufDesc->addBufferBinding(1, *shadingUniformBuffer);
	gbufDesc->allocate(*descriptorPool.get());
	gbufDesc->updateDescriptors();

	matrixDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.sponza, 0));
	matrixDesc->addBufferBinding(0, *matrixUniformBuffer);
	matrixDesc->allocate(*descriptorPool.get());
	matrixDesc->updateDescriptors();

	

	compDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.composition, 0));
	compDesc->addInputAttachmentBinding(0, MyRenderPass::kAttachment_POSITION);
	compDesc->addInputAttachmentBinding(1, MyRenderPass::kAttachment_NORMAL);
	compDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_ALBEDO);
	compDesc->allocate(*descriptorPool.get());
	compDesc->updateDescriptors();

	transDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, shaderLayouts.transparent, 0));
	transDesc->addBufferBinding(0, *transformUniformBuffer);
	transDesc->addBufferBinding(1, *shadingUniformBuffer);
	transDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_POSITION);
	transDesc->allocate(*descriptorPool.get());
	transDesc->updateDescriptors();

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
	matrixUniformBuffer = std::make_unique<SbUniformBuffer<MatrixBufferObject>>(*vulkanBase, swapchain->getSize());
	

	transformUniformBuffer = std::make_unique<SbUniformBuffer<UniformBufferObject>>(*vulkanBase,
		swapchain->getSize());


	skeletonUniformBuffer = std::make_unique<SbUniformBuffer<glm::mat4>>(
		*vulkanBase, swapchain->getSize(), 52 * 5); //room for 5 skeletons in buffer

	shadingUniformBuffer = std::make_unique<SbUniformBuffer<ShadingUBO>>(*vulkanBase, 1, drawables.size());

	for (size_t i = 0; i < drawables.size(); i++)
	{
		ShadingUBO data = { drawables[i].AmbientColor, drawables[i].DiffuseColor, drawables[i].SpecularColor };
		shadingUniformBuffer->writeBufferData(data, i);
	}
	shadingUniformBuffer->copyDataToBufferMemory(*vulkanBase);
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
	commandBuffers.resize(swapchain->swapChainFramebuffers.size());

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

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass->renderPass;
		renderPassInfo.framebuffer = swapchain->swapChainFramebuffers[cmdIdx];
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
					VkDescriptorSet sets[] = { matrixDesc->allocatedDSs[0], currentMaterial->descriptor->allocatedDSs[0] };
					vkCmdBindDescriptorSets(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.opaque.getLayout(), 0, 2, sets, 0, nullptr);
			
					vkCmdDrawIndexed(commandBuffers[cmdIdx], sponza.scene.meshes[meshIdx].indexCount, 1, sponza.scene.meshes[meshIdx].indexBase, 0, 0);
				}
			}
		}


		vkCmdNextSubpass(commandBuffers[cmdIdx], VK_SUBPASS_CONTENTS_INLINE);
		{

			vkCmdBindPipeline(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.composition.handle);
			vkCmdBindDescriptorSets(commandBuffers[cmdIdx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.composition.getLayout(),
				0, 1, &compDesc->allocatedDSs[cmdIdx], 0, NULL);
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

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	//float TicksPerSecond = modelScene->mAnimations[0]->mTicksPerSecond != 0 ?
	//	modelScene->mAnimations[0]->mTicksPerSecond : 25.0f;
	//float TimeInTicks = time * TicksPerSecond;
	//float AnimationTime = fmod(TimeInTicks, modelScene->mAnimations[0]->mDuration);

	float s = 0.5f * std::sinf(time / (3.14f)) + 0.5f;
	mymodel->skeletalAnimationComponent.evaluate(deltaTime, std::vector<float> { s });

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), { 0.02f,0.02f ,0.02f });
	ubo.view = cam.getViewMatrix();
	ubo.proj = cam.getProjectionMatrix();
	memcpy(ubo.boneTransforms, mymodel->skeletalAnimationComponent.transformations.data(),
		sizeof(glm::mat4) * mymodel->skeletalAnimationComponent.transformations.size());
	transformUniformBuffer->writeBufferData(ubo);
	transformUniformBuffer->copyDataToBufferMemory(*vulkanBase, currentImage);


	auto buf = matrixUniformBuffer->getPtr();
	buf->model = ubo.model;
	buf->view = cam.getViewMatrix();
	buf->proj = cam.getProjectionMatrix();
	//matrixUniformBuffer->writeBufferData(buf);	
	matrixUniformBuffer->copyDataToBufferMemory(*vulkanBase, currentImage);

	
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
