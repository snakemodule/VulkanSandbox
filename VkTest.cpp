#include "VkTest.h"


#include "SbVulkanBase.h"
#include "SbSwapchain.h"
#include "TestRenderpass.h"
#include "SbPipelineBuilder.h" //todo remove

#include "DescriptorBuilder.h"

#include "VulkanHelperFunctions.h"

#include "Sponza.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::cout << "basic key callback" << std::endl;
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



void VkTest::createVkGuidePipelines(vk::Device device)
{	
	DSL_cache.init(device);
	descriptor_allocator.init(device);

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	SbPipelineBuilder pipelineBuilder = SbPipelineBuilder();
	pipelineBuilder.shaderReflection(device, "shaders/triangle.vert.spv", "shaders/triangle.frag.spv", 
		DSL_cache, *swapchain, *renderPass, TestRenderpass::kSubpass_GBUF);
	pipelineBuilder.setViewport(WIDTH, HEIGHT);
	
	//finally build the pipeline
	vk::Pipeline trianglePipeline = pipelineBuilder.build_pipeline(device, renderPass->renderPass, 0);
	renderPass->subpasses[TestRenderpass::kSubpass_GBUF].pipelines.push_back({ trianglePipeline, pipelineBuilder.bindings, pipelineBuilder.DSL, pipelineBuilder._pipelineLayout });
	
	//-------------

	pipelineBuilder = SbPipelineBuilder();
	pipelineBuilder.shaderReflection(device, "shaders/compose_test.vert.spv", "shaders/compose_test.frag.spv", DSL_cache, *swapchain, *renderPass, TestRenderpass::kSubpass_COMPOSE);
	pipelineBuilder.setViewport(WIDTH, HEIGHT).setInputAssembly(vk::PrimitiveTopology::eTriangleFan);

	//finally build the pipeline
	vk::Pipeline composePipeline = pipelineBuilder.build_pipeline(device, renderPass->renderPass, 1);
	renderPass->subpasses[TestRenderpass::kSubpass_COMPOSE].pipelines.push_back({ composePipeline, pipelineBuilder.bindings, pipelineBuilder.DSL, pipelineBuilder._pipelineLayout });
		
	composeDescSet.resize(swapchain->getSize());
	for (size_t i = 0; i < swapchain->getSize(); i++)
	{
		DescriptorBuilder::begin(&pipelineBuilder, &descriptor_allocator).build(0, composeDescSet[i], i);
	}
	
		
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



void VkTest::init_sync_structures(vk::Device device)
{
	//create synchronization structures

	vk::FenceCreateInfo fenceCreateInfo;
	//we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	_renderFence = device.createFence(fenceCreateInfo);

	//for the semaphores we don't need any flags
	vk::SemaphoreCreateInfo semaphoreCreateInfo;

	_presentSemaphore = device.createSemaphore(semaphoreCreateInfo);
	_renderSemaphore = device.createSemaphore(semaphoreCreateInfo);
}

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
	std::array<VkClearValue, 2> clearValue{};
	clearValue[0] = { { 0.0f, 0.5f, 0.5f, 1.0f } };
	clearValue[1] = { { 0.0f, 0.5f, 0.5f, 1.0f } };

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
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufPipeline);
		vkCmdDraw(cmd, 3, 1, 0, 0);
	}
	vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
	{
		vk::Pipeline composePipeline = renderPass->getSubpassPipeline(TestRenderpass::kSubpass_COMPOSE, 0);
		vk::PipelineLayout composePipelineLayout = renderPass->getSubpassPipelineLayout(TestRenderpass::kSubpass_COMPOSE, 0);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, composePipeline);

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

void VkTest::initVulkan() {
	vulkanBase = new SbVulkanBase();
	vulkanBase->createInstance(enableValidationLayers);
	vulkanBase->setupDebugMessenger(enableValidationLayers);
	vulkanBase->createSurface(window);
	vulkanBase->pickPhysicalDevice();
	vulkanBase->createLogicalDevice();

	vulkanBase->commandPool = std::make_unique<SbCommandPool>(*vulkanBase);

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

	sponza = new Sponza();
	sponza->load(*vulkanBase);
}

void VkTest::run() {
	initWindow();
	initVulkan();
	while (true)
	{
		draw(vulkanBase->getDevice());
	}
}
