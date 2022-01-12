#pragma once

#include "vulkan/vulkan.hpp"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "iostream"

#include "ResourceManager.h"

#include "Vertex.h"
#include "SbBuffer.h"
#include "SbVulkanBase.h"

#include "VulkanInitializers.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>



//std::string errorString(VkResult errorCode)
//{
//	switch (errorCode)
//	{
//#define STR(r) case VK_ ##r: return #r
//		STR(NOT_READY);
//		STR(TIMEOUT);
//		STR(EVENT_SET);
//		STR(EVENT_RESET);
//		STR(INCOMPLETE);
//		STR(ERROR_OUT_OF_HOST_MEMORY);
//		STR(ERROR_OUT_OF_DEVICE_MEMORY);
//		STR(ERROR_INITIALIZATION_FAILED);
//		STR(ERROR_DEVICE_LOST);
//		STR(ERROR_MEMORY_MAP_FAILED);
//		STR(ERROR_LAYER_NOT_PRESENT);
//		STR(ERROR_EXTENSION_NOT_PRESENT);
//		STR(ERROR_FEATURE_NOT_PRESENT);
//		STR(ERROR_INCOMPATIBLE_DRIVER);
//		STR(ERROR_TOO_MANY_OBJECTS);
//		STR(ERROR_FORMAT_NOT_SUPPORTED);
//		STR(ERROR_SURFACE_LOST_KHR);
//		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
//		STR(SUBOPTIMAL_KHR);
//		STR(ERROR_OUT_OF_DATE_KHR);
//		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
//		STR(ERROR_VALIDATION_FAILED_EXT);
//		STR(ERROR_INVALID_SHADER_NV);
//#undef STR
//	default:
//		return "UNKNOWN_ERROR";
//	}
//}

// Macro to check and display Vulkan return results
//#define VK_CHECK_RESULT(f)																				\
//{																										\
//	VkResult res = (f);																					\
//	if (res != VK_SUCCESS)																				\
//	{																									\
//		std::cout << "Fatal : VkResult is \"" << errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
//		assert(res == VK_SUCCESS);																		\
//	}																									\
//}	

//a material ready to be used in a scene
struct SceneMaterial
{
	std::string name;
	//vkTools::VulkanTexture diffuse;
	//vkTools::VulkanTexture specular;
	//vkTools::VulkanTexture bump;
	std::shared_ptr<SbTextureImage> diffuse = nullptr;
	std::shared_ptr<SbTextureImage> specular = nullptr;
	std::shared_ptr<SbTextureImage> bump = nullptr;
	bool hasAlpha = false;
	bool hasBump = false;
	bool hasSpecular = false;
	VkPipeline pipeline;

	//moved from scenemesh
	VkDescriptorSet descriptorSet;
};

//a mesh ready to be used in a scene, drawablemesh
struct SceneMesh
{
	SbBuffer indexBuffer;

	SbBuffer vertexBuffer;
	
	uint32_t indexCount;
	uint32_t indexBase;


	SceneMaterial* material;
};


// The scene
class Scene 
{
public: 
	std::vector<SceneMaterial> materials;
	std::vector<SceneMesh> meshes;

	//SbVulkanBase* base;
	
	SbBuffer vertexBuffer;
	SbBuffer indexBuffer;	

	void loadMaterials(const aiScene* aScene)
	{
		std::string assetPath = "models/Willems"; //TODO 

		auto& resources = ResourceManager::getInstance();

		materials.resize(aScene->mNumMaterials);

		for (uint32_t i = 0; i < materials.size(); i++)
		{
			materials[i] = {};

			aiString name;
			aScene->mMaterials[i]->Get(AI_MATKEY_NAME, name);
			aiColor3D ambient;
			aScene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
			materials[i].name = name.C_Str();

			// Textures
			aiString texturefile;
			std::string diffuseMapFile;
			
			// Diffuse
			aScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturefile);
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				std::cout << "  Diffuse: \"" << texturefile.C_Str() << "\"" << std::endl;
				std::string fileName = std::string(texturefile.C_Str());
				diffuseMapFile = fileName;
				std::replace(fileName.begin(), fileName.end(), '\\', '/');
				if (!resources.textureExists(fileName))
				{
					materials[i].diffuse = resources.loadTexture2D(fileName, assetPath + fileName);
				}
				else 
				{
					materials[i].diffuse = resources.textures[fileName];
				}
			}
			else
			{
				//std::cout << "  Material has no diffuse, using dummy texture!" << std::endl;
				//std::cout << "  material should have diffuse" << std::endl;
				//materials[i].diffuse = resources.textures->get("dummy.diffuse");
				//assert(false);
			}

			// Specular
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR) > 0)
			{
				aScene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, 0, &texturefile);
				std::cout << "  Specular: \"" << texturefile.C_Str() << "\"" << std::endl;
				std::string fileName = std::string(texturefile.C_Str());
				std::replace(fileName.begin(), fileName.end(), '\\', '/');
				if (!resources.textureExists(fileName)) 
				{
					materials[i].specular = resources.loadTexture2D(fileName, assetPath + fileName);
				}
				else 
				{
					materials[i].specular = resources.textures[fileName];
				}
			}
			else
			{
				std::cout << "  Material has no specular, using dummy texture!" << std::endl;
				//materials[i].specular = resources.textures->get("dummy.specular");
			}

			// Bump (map_bump is mapped to height by assimp)
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS) > 0)
			{
				aScene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &texturefile);
				std::cout << "  Bump: \"" << texturefile.C_Str() << "\"" << std::endl;
				std::string fileName = std::string(texturefile.C_Str());
				std::replace(fileName.begin(), fileName.end(), '\\', '/');
				materials[i].hasBump = true;
				if (!resources.textureExists(fileName))
				{
					materials[i].bump = resources.loadTexture2D(fileName, assetPath + fileName);
				}
				else
				{
					materials[i].bump = resources.textures[fileName];
				}
			}
			else
			{
				std::cout << "  Material has no bump, using dummy texture!" << std::endl;
				//materials[i].specular = resources.textures->get("dummy.specular");
			}

			// Mask
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_OPACITY) > 0)
			{
				std::cout << "  Material has opacity, enabling alpha test" << std::endl;
				materials[i].hasAlpha = true;
			}

			//materials[i].pipeline = resources.pipelines->get("scene.solid");
		}
	}

	void loadMeshes(const aiScene* aScene, SbVulkanBase* base)
	{		
		std::vector<Vertex> gVertices;
		std::vector<uint32_t> gIndices;
		unsigned int gIndexBase = 0;

		VkDevice device = base->getDevice();

		meshes.resize(aScene->mNumMeshes);
		for (uint32_t i = 0; i < meshes.size(); i++)
		{
			aiMesh* aMesh = aScene->mMeshes[i];

			std::cout << "Mesh \"" << aMesh->mName.C_Str() << "\"" << std::endl;
			std::cout << "	Material: \"" << materials[aMesh->mMaterialIndex].name << "\"" << std::endl;
			std::cout << "	Faces: " << aMesh->mNumFaces << std::endl;

			meshes[i].material = &materials[aMesh->mMaterialIndex];
			meshes[i].indexBase = gIndexBase;

			// Vertices
			std::vector<Vertex> vertices;
			vertices.resize(aMesh->mNumVertices);

			bool hasUV = aMesh->HasTextureCoords(0);
			bool hasTangent = aMesh->HasTangentsAndBitangents();

			uint32_t vertexBase = gVertices.size();

			for (uint32_t i = 0; i < aMesh->mNumVertices; i++)
			{		
				vertices[i].pos = glm::make_vec3(&aMesh->mVertices[i].x);// *0.5f;
				vertices[i].pos.y = -vertices[i].pos.y;
				vertices[i].texCoord = (hasUV) ? 
					glm::make_vec2(&aMesh->mTextureCoords[0][i].x) : 
					glm::vec3(0.0f);
				vertices[i].normal = glm::make_vec3(&aMesh->mNormals[i].x);
				vertices[i].normal.y = -vertices[i].normal.y;
				vertices[i].color = glm::vec3(1.0f); // todo : take from material
				vertices[i].tangent = (hasTangent) ? 
					glm::make_vec3(&aMesh->mTangents[i].x) : 
					glm::vec3(0.0f, 1.0f, 0.0f);
				vertices[i].bitangent = (hasTangent) ? 
					glm::make_vec3(&aMesh->mBitangents[i].x) : 
					glm::vec3(0.0f, 1.0f, 0.0f);
				gVertices.push_back(vertices[i]);
			}

			// Indices
			std::vector<uint32_t> indices;
			meshes[i].indexCount = aMesh->mNumFaces * 3;
			indices.resize(aMesh->mNumFaces * 3);
			for (uint32_t i = 0; i < aMesh->mNumFaces; i++)
			{
				// Assume mesh is triangulated
				indices[i * 3] = aMesh->mFaces[i].mIndices[0];
				indices[i * 3 + 1] = aMesh->mFaces[i].mIndices[1];
				indices[i * 3 + 2] = aMesh->mFaces[i].mIndices[2];
				gIndices.push_back(indices[i * 3] + vertexBase);
				gIndices.push_back(indices[i * 3 + 1] + vertexBase);
				gIndices.push_back(indices[i * 3 + 2] + vertexBase);
				gIndexBase += 3;
			}

			// Create buffers
			// todo : staging
			// todo : only one memory allocation
			/*
			uint32_t vertexDataSize = vertices.size() * sizeof(Vertex);
			uint32_t indexDataSize = indices.size() * sizeof(uint32_t);

			VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
			VkMemoryRequirements memReqs;

			VkResult err;
			void* data;
						
			SbBuffer vBuffer = SbBuffer(*base, vertexDataSize, 
				vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible);
			vBuffer.MapAndFill(base->getDevice(), vertices.data(), vertexDataSize);
			
			meshes[i].vertexBuffer = SbBuffer(*base, vertexDataSize, vk::BufferUsageFlagBits::eVertexBuffer
				| vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);


			SbBuffer iBuffer = SbBuffer(*base, indexDataSize, 
				vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible);
			iBuffer.MapAndFill(base->getDevice(), vertices.data(), indexDataSize);



			//vk::BufferUsageFlagBits::eVertexBuffer
				//| vk::BufferUsageFlagBits::eTransferDst


			// Generate vertex buffer
			VkBufferCreateInfo vBufferInfo;

			

			// Staging buffer
			vBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertexDataSize);
			VK_CHECK_RESULT(vkCreateBuffer(device, &vBufferInfo, nullptr, &staging.vBuffer.buffer));
			vkGetBufferMemoryRequirements(device, staging.vBuffer.buffer, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = 
				base->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				//getMemTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &staging.vBuffer.memory));
			VK_CHECK_RESULT(vkMapMemory(device, staging.vBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data));
			memcpy(data, vertices.data(), vertexDataSize);
			vkUnmapMemory(device, staging.vBuffer.memory);
			VK_CHECK_RESULT(vkBindBufferMemory(device, staging.vBuffer.buffer, staging.vBuffer.memory, 0));

			// Target
			vBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertexDataSize);
			VK_CHECK_RESULT(vkCreateBuffer(device, &vBufferInfo, nullptr, &meshes[i].vertexBuffer));
			vkGetBufferMemoryRequirements(device, meshes[i].vertexBuffer, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = base->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &meshes[i].vertexMemory));
			VK_CHECK_RESULT(vkBindBufferMemory(device, meshes[i].vertexBuffer, meshes[i].vertexMemory, 0));

			// Generate index buffer
			VkBufferCreateInfo iBufferInfo;

			// Staging buffer
			iBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indexDataSize);
			VK_CHECK_RESULT(vkCreateBuffer(device, &iBufferInfo, nullptr, &staging.iBuffer.buffer));
			vkGetBufferMemoryRequirements(device, staging.iBuffer.buffer, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = base->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &staging.iBuffer.memory));
			VK_CHECK_RESULT(vkMapMemory(device, staging.iBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data));
			memcpy(data, indices.data(), indexDataSize);
			vkUnmapMemory(device, staging.iBuffer.memory);
			VK_CHECK_RESULT(vkBindBufferMemory(device, staging.iBuffer.buffer, staging.iBuffer.memory, 0));

			// Target
			iBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indexDataSize);
			VK_CHECK_RESULT(vkCreateBuffer(device, &iBufferInfo, nullptr, &meshes[i].indexBuffer));
			vkGetBufferMemoryRequirements(device, meshes[i].indexBuffer, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = base->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &meshes[i].indexMemory));
			VK_CHECK_RESULT(vkBindBufferMemory(device, meshes[i].indexBuffer, meshes[i].indexMemory, 0));

			// Copy
			VkCommandBuffer copyCmd = base->commandPool->beginSingleTimeCommands();
			//VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
			//VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));


			VkBufferCopy copyRegion = {};

			copyRegion.size = vertexDataSize;
			vkCmdCopyBuffer(
				copyCmd,
				staging.vBuffer.buffer,
				meshes[i].vertexBuffer,
				1,
				&copyRegion);

			copyRegion.size = indexDataSize;
			vkCmdCopyBuffer(
				copyCmd,
				staging.iBuffer.buffer,
				meshes[i].indexBuffer,
				1,
				&copyRegion);

			VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &copyCmd;

			base->commandPool->endSingleTimeCommands(copyCmd);
			
			vkDestroyBuffer(device, staging.vBuffer.buffer, nullptr);
			vkFreeMemory(device, staging.vBuffer.memory, nullptr);
			vkDestroyBuffer(device, staging.iBuffer.buffer, nullptr);
			vkFreeMemory(device, staging.iBuffer.memory, nullptr);

			*/
		}


		/* test: global buffers */
		size_t vertexDataSize = gVertices.size() * sizeof(Vertex);
		size_t indexDataSize = gIndices.size() * sizeof(uint32_t);

		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		VkResult err;
		void* data;

		// Generate vertex buffer
		// Staging buffer
		SbBuffer vBuffer = SbBuffer(*base, vertexDataSize, 
			vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible);		
		// Target
		vertexBuffer = SbBuffer(*base, vertexDataSize, vk::BufferUsageFlagBits::eVertexBuffer 
			| vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
				
		// Generate index buffer
		// Staging buffer
		SbBuffer iBuffer = SbBuffer(*base, indexDataSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible);
		// Target
		indexBuffer = SbBuffer(*base, indexDataSize, vk::BufferUsageFlagBits::eIndexBuffer
			| vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		//copy into staging?
		VkCommandBuffer copyCmd = base->commandPool->beginSingleTimeCommands();
		vBuffer.CopyBuffer(copyCmd, vertexBuffer);
		iBuffer.CopyBuffer(copyCmd, indexBuffer);
		base->commandPool->endSingleTimeCommands(copyCmd);

		vBuffer.Destroy(device);// todo: this is ugly, kinda looks like destroying the device
		iBuffer.Destroy(device);

		// Generate descriptor sets for all meshes
		// todo : think about a nicer solution, better suited per material?
		// yes per material

	}

	void load(std::string sceneFilePath, SbVulkanBase* base) 
	{

		Assimp::Importer modelImporter;
		const aiScene* modelScene;

		int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate 
			| aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace 
			| aiProcess_GenSmoothNormals;

		modelScene = modelImporter.ReadFile(sceneFilePath.c_str(),flags);

		loadMaterials(modelScene);

		loadMeshes(modelScene, base);
	}
};