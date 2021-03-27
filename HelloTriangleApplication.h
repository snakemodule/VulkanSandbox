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

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "AnimationStuff.h"

#include "SkeletalAnimationComponent.h"
#include "Model.h"

#include "Header.h"

#include "SbCamera.h"

#include "AnimationKeys.h"
#include "UncompressedAnimationKeys.h"
#include "UncompressedAnimation.h"

//#include "SbCommandPool.h"


//#include "SbDescriptorPool.h"

//#include "SbVulkanBase.h"
//#include "SbSwapchain.h"
#include "SbDescriptorSet.h"

//#include "SbRenderpass.h"
//#include "MyRenderPass.h"

#include "SbUniformBuffer.h"
//#include "SbImage.h"

//#include "SbTextureImage.h"


#include "vulkan/vulkan.hpp"


class SbTextureImage;
class SbVulkanBase;
class SbSwapchain;
class SbRenderpass;
class SbDescriptorPool;
//class SbDescriptorSet;



const int WIDTH = 800;
const int HEIGHT = 600;

const std::string TEXTURE_PATH = "textures/chalet.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;



 

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}




//struct DefaultSkeletonTransformUBO {
//	alignas(16) glm::mat4 boneTransforms[52]; 
//	//this is only the size of the skeleton, the animation system does not output whole skeletons contigously
//};







class HelloTriangleApplication {
public:

	struct DrawableMesh {
		int id;
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


	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::mat4 boneTransforms[52];
	};

	struct ShadingUBO {
		/*alignas(16) glm::vec3 lightPosition;
		alignas(16) glm::vec4 lightColor;
		alignas(16) glm::vec4 specularColor;
		alignas(16) glm::vec4 diffuseColor;
		alignas(16) glm::vec4 ambientColor;*/
		//alignas(256) 
		glm::vec4 specularColor;
		//alignas(256) 
		glm::vec4 diffuseColor;
		//alignas(256) 
		glm::vec4 ambientColor;
	};

	ShadingUBO* shadingUboData;


	HelloTriangleApplication();

	void run();

	~HelloTriangleApplication();

private:
	GLFWwindow* window;

	std::unique_ptr<SbVulkanBase> vulkanBase;
	std::unique_ptr<SbSwapchain> swapchain;
	std::unique_ptr<SbRenderpass> renderPass;

	enum
	{
		kSubpass_GBUF,
		kSubpass_COMPOSE,
		kSubpass_TRANSPARENT,
		kSubpass_COUNT,
		kSubpass_MAX = kSubpass_COUNT - 1
	};


	std::unique_ptr<SbTextureImage> texture;

	vk::Sampler textureSampler;

	//custom model struct
	Model* mymodel = nullptr;


	std::unique_ptr<SbUniformBuffer<UniformBufferObject>> transformUniformBuffer;
	std::unique_ptr<SbUniformBuffer<ShadingUBO>> shadingUniformBuffer;
	std::unique_ptr<SbUniformBuffer<glm::mat4>> skeletonUniformBuffer;

	std::vector<VkBuffer> transformationgUB;
	std::vector<VkDeviceMemory> transformationUBMemory;
	std::unique_ptr<SbDescriptorPool> descriptorPool;
	std::vector<VkCommandBuffer> commandBuffers;

	VkDescriptorSet myOneDescriptorSet;

	std::unique_ptr<SbDescriptorSet> gbufDesc;
	std::unique_ptr<SbDescriptorSet> compDesc;
	std::unique_ptr<SbDescriptorSet> transDesc;

	bool framebufferResized = false;


	SbCamera cam;




	void initWindow();

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	void initVulkan();

	std::chrono::high_resolution_clock::time_point frameTime = std::chrono::high_resolution_clock::now();
	float deltaTime = 0;

	void mainLoop();

	void cleanupSwapChain();
	void cleanup();

	void recreateSwapChain();

	void createInstance();



	void createPipelines();

	void createDescriptorSets();



	void createTextureSampler();

	Assimp::Importer modelImporter;
	const aiScene* modelScene;
	AnimationKeys running;
	AnimationKeys walking;
	//UncompressedAnimationKeys uk;
	//SkeletonAnimation sa()

	void loadModel();

	//TODO make function take model argument?
	void createVertexBuffer();

	void createUniformBuffers();

	void createDescriptorPool();

	void createCommandBuffers();

	//todo move this
	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	static std::vector<char> readFile(const std::string& filename);
};