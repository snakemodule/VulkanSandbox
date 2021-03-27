#include "HelloTriangleApplication.h"

#include "VulkanHelperFunctions.hpp"
#include "VulkanInitializers.hpp"

#include "SbDescriptorPool.h"
//#include "SbDescriptorSet.h"
#include "MyRenderPass.h"
#include "SbTextureImage.h"


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
{

}

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

	vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);

	swapchain = std::make_unique<SbSwapchain>(*vulkanBase);
	swapchain->createSwapChain(vulkanBase->surface, window);

	MyRenderPass* pass = new MyRenderPass(*vulkanBase, *swapchain);
	renderPass = std::unique_ptr<MyRenderPass>(pass);

	swapchain->createFramebuffersForRenderpass(renderPass->renderPass);
		
	texture = std::make_unique<SbTextureImage>(*vulkanBase, TEXTURE_PATH);

	createTextureSampler();
	loadModel();
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
	vulkanBase = std::make_unique<SbVulkanBase>();
	vulkanBase->createInstance(enableValidationLayers);
	vulkanBase->setupDebugMessenger(enableValidationLayers);//moved here from setupdebugmessenger
	vulkanBase->createSurface(window); //moved from createsurface

	vulkanBase->pickPhysicalDevice();
	vulkanBase->createLogicalDevice();
}


void HelloTriangleApplication::createPipelines() {
	//TODO why use index twice?
	auto& subpassgbuf = renderPass->subpasses[kSubpass_GBUF];
	const auto& bind = Vertex::getBindingDescriptions();
	const auto& attr = Vertex::getAttributeDescriptions();
	subpassgbuf.pipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/gbuf.vert.spv", "shaders/gbuf.frag.spv")
		.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE), 0, 3)
		.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
		.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})		
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);

	auto& subpasscomp = renderPass->subpasses[kSubpass_COMPOSE];
	subpasscomp.pipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/composition.vert.spv", "shaders/composition.frag.spv")
		.cullMode(VK_CULL_MODE_NONE)
		.depthWriteEnable(VK_FALSE)
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);

	auto& subpastransparent = renderPass->subpasses[kSubpass_TRANSPARENT];
	subpastransparent.pipeline.shaderLayouts(vulkanBase->getDevice(), "shaders/transparent.vert.spv", "shaders/transparent.frag.spv")
		.colorBlending(0)
		.vertexBindingDescription(std::vector<VkVertexInputBindingDescription> {bind.begin(), bind.end()})
		.vertexAttributeDescription(std::vector<VkVertexInputAttributeDescription> {attr.begin(), attr.end()})
		.cullMode(VK_CULL_MODE_BACK_BIT)
		.depthWriteEnable(VK_FALSE)
		.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device);

}

void HelloTriangleApplication::createDescriptorSets()
{


	gbufDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, renderPass->subpasses[kSubpass_GBUF], 0));
	gbufDesc->addBufferBinding(0, *transformUniformBuffer);
	//VkImageView  v = texture->textureImageView;
	//gbufDesc->addImageBinding(1, textureSampler, &v);
	gbufDesc->addBufferBinding(1, *shadingUniformBuffer);
	gbufDesc->allocate(*descriptorPool.get());
	gbufDesc->updateDescriptors();

	compDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, renderPass->subpasses[kSubpass_COMPOSE], 0));
	compDesc->addInputAttachmentBinding(0, MyRenderPass::kAttachment_POSITION);
	compDesc->addInputAttachmentBinding(1, MyRenderPass::kAttachment_NORMAL);
	compDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_ALBEDO);
	compDesc->allocate(*descriptorPool.get());
	compDesc->updateDescriptors();

	transDesc = std::unique_ptr<SbDescriptorSet>(
		new SbDescriptorSet(vulkanBase->getDevice(), *swapchain, renderPass->subpasses[kSubpass_TRANSPARENT], 0));
	transDesc->addBufferBinding(0, *transformUniformBuffer);
	//v = texture->textureImageView;
	//transDesc->addImageBinding(1, textureSampler, &v);
	transDesc->addBufferBinding(1, *shadingUniformBuffer);
	transDesc->addInputAttachmentBinding(2, MyRenderPass::kAttachment_POSITION);
	transDesc->allocate(*descriptorPool.get());
	transDesc->updateDescriptors();
}


void HelloTriangleApplication::createTextureSampler() {

	auto samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags(),
		vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		0, VK_TRUE, 16,
		VK_FALSE, vk::CompareOp::eAlways,
		0, texture->mipLevels, //todo this sampler is hardcoded to use the miplevels of the one and only texture
		vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

	textureSampler = vulkanBase->getDevice().createSampler(samplerInfo);
}

void HelloTriangleApplication::loadModel() {

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
			mymodel->meshes[i].material.specularColor = { specular.r, specular.g, specular.b, specular.a };
		}
	}


}

//TODO make function take model argument?

void HelloTriangleApplication::createVertexBuffer() {

	using BufferUsage = vk::BufferUsageFlagBits;
	using MemoryProperty = vk::MemoryPropertyFlagBits;

	for (size_t i = 0; i < mymodel->meshes.size(); i++)
	{
		vk::DeviceSize bufferSize = sizeof(Vertex) * mymodel->meshes[i].vertexBuffer.size();
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

void HelloTriangleApplication::createUniformBuffers() {
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

	descriptorPool = std::make_unique<SbDescriptorPool>(vulkanBase->logicalDevice->device);

	std::vector<VkDescriptorPoolSize> poolSizes(4);
	poolSizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(100) };//(swapchain->getSize() * 2) }; 
	poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(100) };//(swapchain->getSize() * 2) };
	poolSizes[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(100) };//(swapchain->getSize() * 2) };
	poolSizes[3] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, static_cast<uint32_t>(100) };//(swapchain->getSize() * 4) };

	descriptorPool->createDescriptorPool(poolSizes, static_cast<uint32_t>(swapchain->getSize()) * 3);

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

		{
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_GBUF));



			for (size_t j = 0; j < drawables.size() - 1; j++)
			{
				uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
					renderPass->getSubpassPipelineLayout(kSubpass_GBUF),
					0, 1, &gbufDesc->allocatedDSs[i], 0, &offset); //todo dynamic offset count should be what?

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
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipelineLayout(kSubpass_COMPOSE), //TODO GET LAYOUT FROM "shaderlayout"
				0, 1, &compDesc->allocatedDSs[i], 0, NULL);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		}


		vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
		{

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipeline(kSubpass_TRANSPARENT));

			for (size_t j = drawables.size() - 1; j < drawables.size(); j++)
			{
				uint32_t offset = j * static_cast<uint32_t>(shadingUniformBuffer->dynamicAlignment);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->getSubpassPipelineLayout(kSubpass_TRANSPARENT),
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

//todo move this



void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage) {

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	float TicksPerSecond = modelScene->mAnimations[0]->mTicksPerSecond != 0 ?
		modelScene->mAnimations[0]->mTicksPerSecond : 25.0f;
	float TimeInTicks = time * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, modelScene->mAnimations[0]->mDuration);
	
	float s = 0.5f * std::sinf(time / (3.14f)) + 0.5f;
	mymodel->skeletalAnimationComponent.evaluate(deltaTime, std::vector<float> { s });

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f))
		//* glm::translate(glm::mat4(1.0f), { 0.0f,-1.0f,0.0f })
		* glm::scale(glm::mat4(1.0f), { 0.01f,0.01f ,0.01f });//* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
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
