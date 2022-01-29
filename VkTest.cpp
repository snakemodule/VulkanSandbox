#include "VkTest.h"


#include "SbVulkanBase.h"
#include "SbSwapchain.h"
#include "TestRenderpass.h"
#include "SbPipelineBuilder.h" //todo remove

#include "DescriptorBuilder.h"

#include "VulkanHelperFunctions.h"

#include "Sponza.h"

unsigned int pressed = 2;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_1:
		pressed = 0;
		break;
	case GLFW_KEY_2:
		pressed = 1;
		break;
	case GLFW_KEY_3:
		pressed = 2;
		break;
	default:
		break;
	}
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VkTest*>(glfwGetWindowUserPointer(window));
	throw std::runtime_error("resize not implemented!");
	//app->framebufferResized = true; //todo
}

void VkTest::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

struct PushConstantBuffer {
	uint32_t mode;
};

struct ModelPushConstant {
	glm::mat4 model;
};

void VkTest::createVkGuidePipelines(vk::Device device)
{	
	DSL_cache.init(device);
	descriptor_allocator.init(device);

	vk::PipelineColorBlendAttachmentState noWrite = vks::initializers::pipelineColorBlendAttachmentState();
	noWrite.colorWriteMask = vk::ColorComponentFlags();

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	SbPipelineBuilder gbufPipeBuilder = SbPipelineBuilder();
	gbufPipeBuilder.shaderReflection(device, "shaders/triangle.vert.spv", "shaders/triangle.frag.spv", 
		DSL_cache, *swapchain, *renderPass, TestRenderpass::kSubpass_GBUF)
		.setViewport(WIDTH, HEIGHT);
	
	//finally build the pipeline
	vk::Pipeline trianglePipeline = gbufPipeBuilder.build_pipeline(device, renderPass->renderPass, 0);
	renderPass->subpasses[TestRenderpass::kSubpass_GBUF].pipelines.push_back(
		{ trianglePipeline, gbufPipeBuilder.bindings, gbufPipeBuilder.DSL, gbufPipeBuilder._pipelineLayout });
	
	//SbUniformBufferSingle<VP> buffer = SbUniformBufferSingle<VP>();		
	//VPUniform.resize(swapchain->getSize());
	//for (size_t i = 0; i < VPUniform.size(); i++)
	//{
	//	VPUniform[i].init(*vulkanBase);
	//	VPUniform[i][0].view = cam.getViewMatrix();
	//	VPUniform[i][0].proj = cam.getProjectionMatrix();
	//	VPUniform[i].copyToDeviceMemory(*vulkanBase);
	//}
	//
	//VPDescSet.resize(swapchain->getSize());
	//for (size_t i = 0; i < swapchain->getSize(); i++)
	//{
	//	vk::DescriptorBufferInfo info;
	//	info.buffer = VPUniform[i].buffer;
	//	info.offset = 0;
	//	info.range = sizeof(VP);
	//
	//	DescriptorBuilder::begin(&gbufPipeBuilder, &descriptor_allocator)
	//		.bind_buffer(0, &info, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
	//		.build(0, VPDescSet[i], i);
	//}

	//-------------

	SbPipelineBuilder composePipeBuilder = SbPipelineBuilder();
	composePipeBuilder.shaderReflection(device, "shaders/compose_test.vert.spv", "shaders/compose_test.frag.spv",
		DSL_cache, *swapchain, *renderPass, TestRenderpass::kSubpass_COMPOSE)
		.setViewport(WIDTH, HEIGHT).setInputAssembly(vk::PrimitiveTopology::eTriangleFan);

	//finally build the pipeline
	vk::Pipeline composePipeline = composePipeBuilder.build_pipeline(device, renderPass->renderPass, 1);
	renderPass->subpasses[TestRenderpass::kSubpass_COMPOSE].pipelines.push_back(
		{ composePipeline, composePipeBuilder.bindings, composePipeBuilder.DSL, composePipeBuilder._pipelineLayout });

	composeDescSet.resize(swapchain->getSize());
	for (size_t i = 0; i < swapchain->getSize(); i++)
	{
		DescriptorBuilder::begin(&composePipeBuilder, &descriptor_allocator).build(0, composeDescSet[i], i);
	}
	
	//----------------
	
	//sponza = new Sponza();
	//sponza->load(*vulkanBase);
	//sponza->scene.prepareMaterialDescriptors(*vulkanBase, descriptor_allocator, gbufPipeBuilder.DSL[1]);
		
}

void VkTest::createSbPipelines(vk::Device device)
{
	//auto& defaultPipelines = renderPass->subpasses[TestRenderpass::kSubpass_GBUF].pipelines;
	//
	//defaultPipelines.emplace_back().shaderLayouts(vulkanBase->getDevice(), "shaders/gbuf.vert.spv", "shaders/gbuf.frag.spv")
	//	.addBlendAttachmentStates(vks::initializers::pipelineColorBlendAttachmentState())
	////	.vertexBindingDescription(std::vector<vk::VertexInputBindingDescription> {bind.begin(), bind.end()})
	////	.vertexAttributeDescription(std::vector<vk::VertexInputAttributeDescription> {attr.begin(), attr.end()})
	//	.createPipeline(renderPass->renderPass, vulkanBase->logicalDevice->device, TestRenderpass::kSubpass_GBUF); //todo this enum should be automatic
}



//void VkTest::init_sync_structures(vk::Device device)
//{
//	//create synchronization structures
//
//	vk::FenceCreateInfo fenceCreateInfo;
//	//we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
//	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
//
//	_renderFence = device.createFence(fenceCreateInfo);
//
//	//for the semaphores we don't need any flags
//	vk::SemaphoreCreateInfo semaphoreCreateInfo;
//
//	_presentSemaphore = device.createSemaphore(semaphoreCreateInfo);
//	_renderSemaphore = device.createSemaphore(semaphoreCreateInfo);
//}

void VkTest::createCommandBuffers(vk::Device device) {
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.commandPool = vulkanBase->commandPool->handle;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t)swapchain->swapChainFramebuffers.size();

	commandBuffers = device.allocateCommandBuffers(allocInfo);
}

void VkTest::draw(vk::Device device)
{
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	//device.waitForFences(1, &_renderFence, true, 1000000000);
	//device.resetFences(1, &_renderFence);
	uint32_t imageIndex = swapchain->acquireNextImage();
	//std::vector<vk::CommandBuffer> cmds{ commandBuffers[imageIndex] };

	//naming it cmd for shorter writing
	vk::CommandBuffer cmd = commandBuffers[imageIndex];

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	std::array<VkClearValue, 4> clearValue{};
	clearValue[0] = { { 0.0f, 0.5f, 0.5f, 1.0f } };
	clearValue[1] = { { 0.0f, 0.5f, 0.5f, 1.0f } };
	clearValue[2] = { { 0.0f, 0.5f, 0.5f, 1.0f } };
	clearValue[3] = { { 0.0f, 0.5f, 0.5f, 1.0f } };

	//float flash = abs(sin(_frameNumber / 120.f));
	//clearValue[0].color = { { 0.0f, 0.5f, 0.5f, 1.0f } };

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = renderPass->renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = vk::Extent2D(WIDTH, HEIGHT);
	rpInfo.framebuffer = swapchain->swapChainFramebuffers[imageIndex];

	//connect clear values
	/*
	Validation Error: [ VUID-VkRenderPassBeginInfo-clearValueCount-00902 ] Object 0: handle = 0x9f9b41000000003c, type = VK_OBJECT_TYPE_RENDER_PASS; | MessageID = 0x4de051e3 |
	In vkCmdBeginRenderPass() the VkRenderPassBeginInfo struct has a clearValueCount of 1 but there must be at least 2 entries in pClearValues array to account for the highest index
	attachment in VkRenderPass 0x9f9b41000000003c[] that uses VK_ATTACHMENT_LOAD_OP_CLEAR is 2. Note that the pClearValues array is indexed by attachment number so even if some pClearValues
	entries between 0 and 1 correspond to attachments that aren't cleared they will be ignored. The Vulkan spec states: clearValueCount must be greater than the largest attachment index in
	renderPass that specifies a loadOp (or stencilLoadOp, if the attachment has a depth/stencil format) of VK_ATTACHMENT_LOAD_OP_CLEAR
	(https://vulkan.lunarg.com/doc/view/1.2.189.2/windows/1.2-extensions/vkspec.html#VUID-VkRenderPassBeginInfo-clearValueCount-00902)
	*/
	rpInfo.clearValueCount = clearValue.size(); // clear
	rpInfo.pClearValues = clearValue.data();

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		vk::Pipeline gbufPipeline = renderPass->getSubpassPipeline(TestRenderpass::kSubpass_GBUF, 0);
		vk::PipelineLayout gbufPipelineLayout = renderPass->getSubpassPipelineLayout(TestRenderpass::kSubpass_GBUF);

		VkDeviceSize offsets[] = { 0 };
		//vkCmdBindVertexBuffers(cmd, 0, 1, &sponza->scene.vertexBuffer.buffer, offsets);
		//vkCmdBindIndexBuffer(cmd, sponza->scene.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufPipeline);
		//VkDescriptorSet VP = VPDescSet[imageIndex];
		//vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufPipelineLayout, 0, 1, &VP, 0, NULL);
		//for (auto mesh : sponza->scene.meshes)
		//{
		//	if (sponza->scene.materials.hasAlpha[mesh.material])
		//	{
		//		continue;
		//	}
			//VkDescriptorSet materialDescriptorSet = sponza->scene.materials.descriptorSets[mesh.material];
			//vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufPipelineLayout, 1, 1, &materialDescriptorSet, 0, NULL);
			//glm::mat4 identity = glm::mat4(1.0f);
			//vkCmdPushConstants(cmd, gbufPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelPushConstant), &identity);
			//vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, mesh.indexBase, 0);
		//}
		
		vkCmdDraw(cmd, 3, 1, 0, 0);
	}
	vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
	{
		vk::Pipeline composePipeline = renderPass->getSubpassPipeline(TestRenderpass::kSubpass_COMPOSE, 0);
		vk::PipelineLayout composePipelineLayout = renderPass->getSubpassPipelineLayout(TestRenderpass::kSubpass_COMPOSE);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, composePipeline);

		PushConstantBuffer constants;
		constants.mode = pressed;
		//upload to the GPU via push constants
		vkCmdPushConstants(cmd, composePipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBuffer), &constants);

		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, composePipelineLayout, 0, 1, &composeDescSet[imageIndex], 0, nullptr);
		vkCmdDraw(cmd, 4, 1, 0, 0);
	}
	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	vkEndCommandBuffer(cmd);

	std::vector<vk::CommandBuffer> cmds{ cmd };
	std::vector<vk::Semaphore> waitSem{ swapchain->getImageAvailableSemaphore() };
	std::vector<vk::Semaphore> signalSem{ swapchain->getRenderFinishedSemaphore() };
	vulkanBase->submitCommandBuffers(cmds, waitSem, signalSem, swapchain->getInFlightFence());
	swapchain->presentImage(imageIndex, signalSem);
	swapchain->updateFrameInFlightCounter();
}

void createTextureSampler()
{
	
}

void VkTest::initVulkan() {
	vulkanBase = new SbVulkanBase();
	vulkanBase->createInstance(enableValidationLayers);
	vulkanBase->setupDebugMessenger(enableValidationLayers);
	vulkanBase->createSurface(window);
	vulkanBase->pickPhysicalDevice();
	vulkanBase->createLogicalDevice();

	vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);

	ResourceManager::getInstance().init(vulkanBase);
	auto samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags(),
		vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		0, VK_TRUE, 16,
		VK_FALSE, vk::CompareOp::eAlways,
		0, 1, //todo do we need many many samplers to allow for more lod/mips?
		vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

	ResourceManager::getInstance().createTextureSampler("shared sampler", samplerInfo);

	swapchain = new SbSwapchain(*vulkanBase);
	swapchain->createSwapChain(vulkanBase->surface, window);//make oneliner
		
	renderPass = new TestRenderpass(*vulkanBase, *swapchain);

	swapchain->createFramebuffersForRenderpass(renderPass->renderPass);
		
	//createVertexBuffer();

	//createUniformBuffers();
	//createDescriptorPool();

	createVkGuidePipelines(vulkanBase->getDevice());
	//createDescriptorSets();

	

	createCommandBuffers(vulkanBase->getDevice());
	swapchain->createSyncObjects(MAX_FRAMES_IN_FLIGHT);

	
}

void VkTest::run() {
	initWindow();
	initVulkan();
	while (true)
	{
		glfwPollEvents();
		draw(vulkanBase->getDevice());
	}
}
