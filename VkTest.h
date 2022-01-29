#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "vulkan/vulkan.hpp"



class SbVulkanBase;
class SbSwapchain;

class SbRenderpass;

class Sponza;

#include "DescriptorSetLayoutCache.h"
#include "DescriptorAllocator.h"

#include "SbUniformBufferSingle.h"
#include "SbCamera.h"

#include "glm/ext/matrix_float4x4.hpp"  // for mat4

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void framebufferResizeCallback(GLFWwindow* window, int width, int height);

struct VP {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class VkTest
{
public:
	GLFWwindow* window;
	const int WIDTH = 800;
	const int HEIGHT = 600;
	const int MAX_FRAMES_IN_FLIGHT = 2;

	SbCamera cam = SbCamera(WIDTH, HEIGHT);

	bool framebufferResized = false;

	std::vector<vk::DescriptorSet> composeDescSet;
	std::vector<vk::DescriptorSet> VPDescSet;

	std::vector<SbUniformBufferSingle<VP>> VPUniform;

	void initWindow();

	SbVulkanBase* vulkanBase;
	SbSwapchain* swapchain;
	SbRenderpass* renderPass;
	vk::PipelineLayout _trianglePipelineLayout;
	//vk::Pipeline _trianglePipeline;
	DescriptorSetLayoutCache DSL_cache;
	DescriptorAllocator descriptor_allocator;

	Sponza* sponza = nullptr;

	void createVkGuidePipelines(vk::Device device);

	void createSbPipelines(vk::Device device);
	
	//main loop
	//vk::Semaphore _presentSemaphore, _renderSemaphore;
	//vk::Fence _renderFence;
	std::vector<vk::CommandBuffer> commandBuffers;

	void init_sync_structures(vk::Device device);

	void createCommandBuffers(vk::Device device);

	void draw(vk::Device device);

	void initVulkan();	

	void run();

	VkTest() {}

	~VkTest() {}

};

