#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

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

#include "AnimationStuff.h"
#include "SkeletalAnimationComponent.h"
#include "Model.h"

#include "SbCamera.h"
#include "UncompressedAnimationKeys.h"
#include "UncompressedAnimation.h"
#include "SbDescriptorSet.h"
#include "SbUniformBuffer.h"
#include "SbPipeline.h"
#include "SbComputePipeline.h"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"

#include "SbFramebuffer.h"

class SbTextureImage;
class SbVulkanBase;
class SbSwapchain;
class SbRenderpass;
class SbDescriptorPool;
class RenderPassHelper;


const int WIDTH = 800;
const int HEIGHT = 600;

const std::string TEXTURE_PATH = "textures/chalet.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;


namespace std {
	template<> struct hash<AnimatedVertex> {
		size_t operator()(AnimatedVertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct SbAllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;

};


class HelloTriangleApplication {
public:

	VmaAllocator vmaAllocator;

	//SbTextureImage* shadowCubeMap;

	struct {
		VkImage image;
		unsigned width, height;
		VkDeviceMemory deviceMemory;
		VkSampler sampler;
		VkImageView view;
	} shadowCubeMap;

	struct {
		SbPipeline opaque;
		SbPipeline masked;

		SbPipeline composition;

		SbComputePipeline cluster;
		SbComputePipeline lightAssignment;

		SbPipeline shadow; //todo create pipeline

		//SbPipeline character;
		//SbPipeline transparentCharacter;
	} pipelines;

	struct
	{
		SbShaderLayout gbuf;
		SbShaderLayout sponza;
		SbShaderLayout composition;

		SbShaderLayout light_composition;

		SbShaderLayout transparent;

		SbShaderLayout cluster;
		SbShaderLayout lightAssignment;

		SbShaderLayout shadow;
	} shaderLayouts;

	struct DrawableMesh {
		VkBuffer VertexBuffer;
		VkDeviceMemory VertexBufferMemory;
		VkBuffer IndexBuffer;
		VkDeviceMemory IndexBufferMemory;
		uint32_t IndexCount = 0;
		uint32_t DynamicBufferOffset = 0;
		glm::vec4 AmbientColor;
		glm::vec4 DiffuseColor;
		glm::vec4 SpecularColor;
		float Shininess;
	};
	std::vector<DrawableMesh> drawables;

	struct LightGrid {
		uint32_t offset;
		uint32_t count;
	};

	struct PointLight
	{
		glm::vec4 position;
		glm::vec4 color;
		uint32_t enabled;
		float intensity;
		float range;
	};

	struct AABB {
		glm::vec4 min;
		glm::vec4 max;
	};

	struct ScreenToViewUniform {
		glm::mat4 inverseProjection;
		glm::uvec4 tileSizes;
		glm::uvec2 screenDimensions;
		float sliceScalingFactor;
		float sliceBiasFactor;
	};

	struct MatrixBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::mat4 boneTransforms[52];
	};

	struct ShadingUBO {
		glm::vec4 specularColor;
		glm::vec4 diffuseColor;
		glm::vec4 ambientColor;
	};

	HelloTriangleApplication();

	void run();

	~HelloTriangleApplication();

private:
	GLFWwindow* window = nullptr;

	std::unique_ptr<SbVulkanBase> vulkanBase;

	SbSwapchain* swapchain;

	struct deferred_subpasses {
		enum {
			GBUF,
			COMPOSE,
			COUNT
		};
	};

	struct deferred_attachments {
		enum {
			BACK,
			POSITION,
			NORMAL,
			ALBEDO,
			DEPTH,
			COUNT
		};
	};


	std::vector<SbFramebuffer> deferredFrameBuffers = {};
	RenderpassHelper* deferredRenderPass;

	struct shadow_subpasses {
		enum {
			DEPTH,
			COUNT
		};
	};

	struct	shadow_attachments {
		enum {
			DEPTH,
			COUNT
		};
	};

	std::vector<SbFramebuffer> shadowFrameBuffers = {};
	RenderpassHelper* shadowRenderPass;

	//std::unique_ptr<SbSwapchain> swapchain;

	vk::Sampler textureSampler;

	AnimatedModel* mymodel = nullptr;

	struct {
		SbUniformBuffer<AABB>* clusterSSBO;
		SbUniformBuffer<ScreenToViewUniform>* screenToViewUniform;
		SbUniformBuffer<PointLight>* lightsSSBO;
		SbUniformBuffer<uint32_t>* lightIndexSSBO;
		SbUniformBuffer<LightGrid>* lightGridSSBO;
		SbUniformBuffer<uint32_t>* lightIndexCountSSBO;

		SbUniformBuffer<MatrixBufferObject>* matrixUniform;
		//SbUniformBuffer<UniformBufferObject>* transformUniform;
		//SbUniformBuffer<ShadingUBO>* shadingUniform;
		SbUniformBuffer<glm::mat4>* skeletonUniform;

		SbUniformBuffer<glm::vec4>* cameraUniform;
	} shaderStorage;

	std::unique_ptr<SbDescriptorPool> descriptorPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandBuffer computeCommandBuffer;

	struct {
		//compute
		SbDescriptorSet* lights;

		SbDescriptorSet* lightAssignment;
		SbDescriptorSet* cluster;

		SbDescriptorSet* lights_g;
		//SbDescriptorSet* compose;

		SbDescriptorSet* matrixDesc;

		SbDescriptorSet* gbufDesc;
		SbDescriptorSet* compDesc;
		SbDescriptorSet* transDesc;
	} descriptorSets;

	bool framebufferResized = false;
	SbCamera cam;

	void initWindow();


	static glm::dvec2 last_mouse_pos;
	static glm::dvec2 delta_mouse_pos;
	static std::vector<int> keyPresses;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	void initVulkan();

	void createShadowRenderpass();

	void createDeferredRenderpass();

	std::chrono::high_resolution_clock::time_point frameTime = std::chrono::high_resolution_clock::now();
	float deltaTime = 0;

	void mainLoop();

	void cleanupSwapChain();
	void cleanup();

	void recreateSwapChain();

	void createInstance();


	void handleKeyPresses();
	void setOutputMode(uint32_t);

	void createPipelines();

	SbAllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	void prepareComputePipeline();

	void createDescriptorSets();

	void createTextureSampler();


	void createVertexBuffer(); //for animated model

	void createUniformBuffers();

	void prepareCubeMap();

	void createOffscreenRenderpass();

	void prepareOffscreenFramebuffer();

	void prepareOffscreenRenderpass();

	void setupDescriptorSetLayout();

	void updateCubeFace(uint32_t faceIndex, VkCommandBuffer commandBuffer);

	void createDescriptorPool();

	void createCommandBuffers();

	//todo move this
	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	static std::vector<char> readFile(const std::string& filename);
};