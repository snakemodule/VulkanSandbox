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



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void framebufferResizeCallback(GLFWwindow* window, int width, int height);

class VkTest
{
public:
	GLFWwindow* window;
	const int WIDTH = 800;
	const int HEIGHT = 600;
	const int MAX_FRAMES_IN_FLIGHT = 2;

	bool framebufferResized = false;

	std::vector<vk::DescriptorSet> composeDescSet;

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
	vk::Semaphore _presentSemaphore, _renderSemaphore;
	vk::Fence _renderFence;
	std::vector<vk::CommandBuffer> commandBuffers;

	void init_sync_structures(vk::Device device);

	void createCommandBuffers(vk::Device device);

	void draw(vk::Device device);

	void initVulkan();	

	void run();

	VkTest() {}

	~VkTest() {}

};

