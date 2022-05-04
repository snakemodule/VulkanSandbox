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


const int WIDTH = 800*2;
const int HEIGHT = 600*2;

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
		std::vector<VkImage> images;
		unsigned width, height;
		std::vector<VkDeviceMemory> deviceMemory;
		VkSampler sampler;		
		std::vector<VkImageView> views;
	} shadowCubeMap;

	struct {
		SbPipeline opaque;
		SbPipeline masked;

		SbPipeline composition;

		SbComputePipeline cluster;
		SbComputePipeline lightAssignment;

		SbPipeline shadow; //todo create pipeline

		SbPipeline debugCube;
		SbPipeline virtualCube;

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
		SbShaderLayout debugCube;
		SbShaderLayout virtualCube;
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

	struct ShadowUBO
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 lightPos;
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

	struct RayCamera {
		glm::mat4 cameraRotation;
		glm::uvec2 screenDimensions;
		float yfovRad;
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
			COLOR,
			DEPTH,
			COUNT
		};
	};

	std::vector<SbFramebuffer> shadowFrameBuffers = {};
	RenderpassHelper* shadowRenderPass;

	std::vector<SbFramebuffer> cubedebugFrameBuffers = {};
	RenderpassHelper* cubedebugRenderPass;
	//RenderpassHelper* virtualCubeRenderPass;

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
		SbUniformBuffer<ShadowUBO>* shadowMatrixUniform;
		//SbUniformBuffer<UniformBufferObject>* transformUniform;
		//SbUniformBuffer<ShadingUBO>* shadingUniform;
		SbUniformBuffer<glm::mat4>* skeletonUniform;

		SbUniformBuffer<glm::vec4>* cameraUniform;

		SbUniformBuffer<RayCamera>* rayCameraUniform;
	} shaderStorage;

	glm::vec4 startLightPos = { -1.0, 2.0, 0.5, 1 };
	glm::vec4 shadowLightPos = { -1.0, 2.0, 0.5, 1 };

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
		SbDescriptorSet* shadowDesc;

		SbDescriptorSet* gbufDesc;
		SbDescriptorSet* compDesc;
		SbDescriptorSet* transDesc;

		SbDescriptorSet* cube;
		SbDescriptorSet* virtualcube;
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
	void createCubeDebugRenderpass();

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

	void createDescriptorSets();

	void createTextureSampler();


	void createVertexBuffer(); //for animated model

	void createUniformBuffers();

	void prepareCubeMap();

	void prepareOffscreenFramebuffer();

	void prepareOffscreenRenderpass();

	void setupDescriptorSetLayout();

	void updateCubeFace(uint32_t faceIndex, size_t cmdIdx);

	void createDescriptorPool();

	void createCommandBuffers();

	void recordCubeMapRenderpass(size_t idx);
	void recordVirtualCubeRenderpass(size_t idx);

	void recordDeferredRenderpass(size_t idx);
	void drawComposition(size_t idx);
	void drawGBUF(size_t idx);

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	static std::vector<char> readFile(const std::string& filename);
};