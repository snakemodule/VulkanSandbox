#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

#include <chrono>
#include <cstdlib>








#include "SbCamera.h"

//#include "AnimationKeys.h"




//#include "SbCommandPool.h"
//#include "SbDescriptorPool.h"
//#include "SbVulkanBase.h"
//#include "SbSwapchain.h"
//#include "SbRenderpass.h"
//#include "MyRenderPass.h"
//#include "SbImage.h"
//#include "SbTextureImage.h"


#include "vulkan/vulkan.hpp"
#include "glm/ext/matrix_float4x4.hpp"  // for mat4
#include "glm/ext/vector_float4.hpp"    // for vec4
#include "stdint.h"                     // for uint32_t
#include "type_traits"                  // for hash
#include "vulkan/vulkan_core.h"         // for VkBuffer, VkDeviceMemory, VkDescriptorSet
#include "xstring"                      // for string
#include "vector"                       // for vector
#include <memory>                       // for allocator


class Sponza;
struct AnimatedModel;
struct AnimatedVertex;
class SbVulkanBase;
class SbSwapchain;
class SbRenderpass;
class SbDescriptorSet;
class SbDescriptorPool;

template <class T>
class SbUniformBuffer;


const int WIDTH = 800;
const int HEIGHT = 600;

const std::string TEXTURE_PATH = "textures/chalet.jpg";
const int MAX_FRAMES_IN_FLIGHT = 2;
 

//amespace std {
//	template<> struct hash<AnimatedVertex> {
//		size_t operator()(AnimatedVertex const& vertex) const {
//			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
//		}
//	};
//

//struct DefaultSkeletonTransformUBO {
//	alignas(16) glm::mat4 boneTransforms[52]; 
//	//this is only the size of the skeleton, the animation system does not output whole skeletons contigously
//};

class HelloTriangleApplication {
public:

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


	struct VPTransformBuffer {
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

	SbVulkanBase* vulkanBase;
	SbSwapchain* swapchain;
	SbRenderpass* renderPass;
	SbDescriptorPool* descriptorPool;


	//std::unique_ptr<SbTextureImage> texture;

	vk::Sampler textureSampler;
	vk::Sampler materialSampler;

	//custom model struct
	AnimatedModel* mymodel = nullptr;

	Sponza* sponza = nullptr;

	SbUniformBuffer<VPTransformBuffer>* vpTransformBuffer;	
	SbUniformBuffer<UniformBufferObject>* transformUniformBuffer;
	SbUniformBuffer<ShadingUBO>* shadingUniformBuffer;

	std::vector<VkBuffer> transformationgUB;
	std::vector<VkDeviceMemory> transformationUBMemory;
	std::vector<VkCommandBuffer> commandBuffers;

	VkDescriptorSet myOneDescriptorSet;

	SbDescriptorSet* gbufDesc;
	SbDescriptorSet* compDesc;
	SbDescriptorSet* transDesc;
	SbDescriptorSet* sceneGlobalDesc;
	SbDescriptorSet* sceneMaterialDesc;
	SbDescriptorSet* sceneInstanceDesc;

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
	void createVertexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createCommandBuffers();

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	static std::vector<char> readFile(const std::string& filename);
};