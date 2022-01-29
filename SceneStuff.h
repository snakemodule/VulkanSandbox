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

#include "SbMaterialDescriptorSet.h"

#include "DescriptorAllocator.h"


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

struct TextureAOS {
	std::vector<vk::ImageView> imageView;
	
	std::vector<vk::Image> image;
	std::vector<vk::DeviceMemory> memory;
	std::vector<vk::DeviceSize> imageSize;
	std::vector<uint32_t> mipLevels;
	std::vector<vk::ImageCreateInfo> imageCreateInfo;
	
	std::vector<vk::DescriptorImageInfo> descriptorInfo;
};

struct SceneMaterialAOS 
{
	std::vector<std::string> name;	
	
	TextureAOS diffuse;
	TextureAOS specular;
	TextureAOS bump;
	std::vector<bool> hasAlpha;	
	std::vector<vk::DescriptorSet> descriptorSets;
};


struct SceneMesh
{
	uint32_t indexCount;
	uint32_t indexBase;
	uint32_t material;
};

// The scene
class Scene 
{
public: 
	SceneMaterialAOS materials;

	//std::vector<SceneMaterial> materials;
	SbMaterialDescriptorSet materialDescriptors;

	std::vector<SceneMesh> meshes;

	//SbVulkanBase* base;
	
	SbBuffer vertexBuffer;
	SbBuffer indexBuffer;	

	void resize(TextureAOS* texture, uint32_t size) {
		texture->imageView.resize(size);		
		texture->image.resize(size);
		texture->memory.resize(size);
		texture->imageSize.resize(size);
		texture->mipLevels.resize(size);
		texture->imageCreateInfo.resize(size);		
		//texture->channels.resize(size);
		texture->descriptorInfo.resize(size);
	}

	void resize(SceneMaterialAOS* materials, uint32_t size) {
		materials->name.resize(size);
		resize(&materials->diffuse, size);
		resize(&materials->specular, size);
		resize(&materials->bump, size);
		materials->hasAlpha.resize(size);
	}
	
	void loadMaterials(const aiScene* aScene, SbVulkanBase* base)
	{
		std::string assetPath = "models/Willems"; //TODO 

		auto& resources = ResourceManager::getInstance();
		VkSampler sharedSampler = resources.samplers["shared sampler"];

		// Add dummy textures for objects without texture
		resources.loadTexture2D("dummy.diffuse", assetPath + "/sponza/dummy.dds", sharedSampler);
		resources.loadTexture2D("dummy.specular", assetPath + "/sponza/dummy_specular.dds", sharedSampler);
		resources.loadTexture2D("dummy.bump", assetPath + "/sponza/dummy_ddn.dds", sharedSampler);

		//materials.resize(aScene->mNumMaterials);
		resize(&materials, aScene->mNumMaterials);

		for (uint32_t i = 0; i < materials.name.size(); i++)
		{
			//materials[i] = {};

			aiString name;
			aScene->mMaterials[i]->Get(AI_MATKEY_NAME, name);
			aiColor3D ambient;
			aScene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, ambient);

			materials.name[i] = name.C_Str();

			// Textures
			aiString texturefile;
			std::string diffuseMapFile;
			
			// Diffuse
			std::shared_ptr<Texture> texture;
			aScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturefile);
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				std::cout << "  Diffuse: \"" << texturefile.C_Str() << "\"" << std::endl;
				std::string fileName = std::string(texturefile.C_Str());
				diffuseMapFile = fileName;
				std::replace(fileName.begin(), fileName.end(), '\\', '/');
				
				if (!resources.textureExists(fileName))
					texture = resources.loadTexture2D(fileName, assetPath + fileName, sharedSampler);
				else
					texture = resources.textures[fileName];				
			}
			else
			{
				std::cout << "  Material has no diffuse, using dummy texture!" << std::endl;
				texture = resources.textures["dummy.diffuse"];
			}
			materials.diffuse.imageView[i] = texture->imageView;
			materials.diffuse.image[i] = texture->image;
			materials.diffuse.memory[i] = texture->memory;
			materials.diffuse.imageSize[i] = texture->imageSize;
			materials.diffuse.imageCreateInfo[i] = texture->imageCreateInfo;
			materials.diffuse.descriptorInfo[i] = texture->descriptorInfo;

			// Specular
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR) > 0)
			{
				aScene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, 0, &texturefile);
				std::cout << "  Specular: \"" << texturefile.C_Str() << "\"" << std::endl;
				std::string fileName = std::string(texturefile.C_Str());
				std::replace(fileName.begin(), fileName.end(), '\\', '/');

				if (!resources.textureExists(fileName))
					texture = resources.loadTexture2D(fileName, assetPath + fileName, sharedSampler);
				else
					texture = resources.textures[fileName];
			}
			else
			{
				std::cout << "  Material has no specular, using dummy texture!" << std::endl;
				texture = resources.textures["dummy.specular"];
			}
			materials.specular.imageView[i] = texture->imageView;
			materials.specular.image[i] = texture->image;
			materials.specular.memory[i] = texture->memory;
			materials.specular.imageSize[i] = texture->imageSize;
			materials.specular.imageCreateInfo[i] = texture->imageCreateInfo;
			materials.specular.descriptorInfo[i] = texture->descriptorInfo;

			// Bump (map_bump is mapped to height by assimp)
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS) > 0)
			{
				aScene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, 0, &texturefile);
				std::cout << "  Bump: \"" << texturefile.C_Str() << "\"" << std::endl;
				std::string fileName = std::string(texturefile.C_Str());
				std::replace(fileName.begin(), fileName.end(), '\\', '/');
				//materials.hasBump[i] = true;
				if (!resources.textureExists(fileName))
					texture = resources.loadTexture2D(fileName, assetPath + fileName, sharedSampler);
				else
					texture = resources.textures[fileName];
			}
			else
			{
				std::cout << "  Material has no bump, using dummy texture!" << std::endl;
				texture = resources.textures["dummy.bump"];
			}
			materials.bump.imageView[i] = texture->imageView;
			materials.bump.image[i] = texture->image;
			materials.bump.memory[i] = texture->memory;
			materials.bump.imageSize[i] = texture->imageSize;
			materials.bump.imageCreateInfo[i] = texture->imageCreateInfo;
			materials.bump.descriptorInfo[i] = texture->descriptorInfo;

			// Mask
			if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_OPACITY) > 0)
			{
				std::cout << "  Material has opacity, enabling alpha test" << std::endl;
				materials.hasAlpha[i] = true;
			}
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
			std::cout << "	Material: \"" << materials.name[aMesh->mMaterialIndex] << "\"" << std::endl;
			std::cout << "	Faces: " << aMesh->mNumFaces << std::endl;

			meshes[i].material = aMesh->mMaterialIndex;// &materials[aMesh->mMaterialIndex];
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
					glm::make_vec2(&aMesh->mTextureCoords[0][i].x) : glm::vec2(0.0f);
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
		}


		/* test: global buffers */
		size_t vertexDataSize = gVertices.size() * sizeof(Vertex);
		size_t indexDataSize = gIndices.size() * sizeof(uint32_t);

		VkMemoryAllocateInfo memAlloc = vk::MemoryAllocateInfo();
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
		
	}
	
	void load(std::string sceneFilePath, SbVulkanBase* base) 
	{
		Assimp::Importer modelImporter;
		const aiScene* modelScene;

		int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate 
			| aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace 
			| aiProcess_GenSmoothNormals;

		modelScene = modelImporter.ReadFile(sceneFilePath.c_str(),flags);

		loadMaterials(modelScene, base);
		loadMeshes(modelScene, base);
	}

	//allocate descriptors for materials
	void allocateMaterialDescriptors(DescriptorAllocator& allocator, vk::DescriptorSetLayout layout, 
		size_t count, std::vector<vk::DescriptorSet>& descriptorSets)
	{
		descriptorSets.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			allocator.allocate(layout, &descriptorSets[i]);
		}

		//std::vector<vk::DescriptorSetLayout> layouts = std::vector<vk::DescriptorSetLayout>(count, layout);
		//vk::DescriptorSetAllocateInfo allocInfo = {};
		//allocInfo.descriptorPool = descriptorPool.pool;
		//allocInfo.descriptorSetCount = layouts.size();
		//allocInfo.pSetLayouts = layouts.data();
		//
		//if (device.allocateDescriptorSets(&allocInfo, descriptorSets.data()) != vk::Result::eSuccess) {
		//	throw std::runtime_error("failed to allocate descriptor sets!");
		//}
	}

	void writeMaterialDescriptors(vk::Device device, uint32_t binding, std::vector<vk::DescriptorImageInfo> imageInfo,
		vk::DescriptorType descriptorType, std::vector<vk::DescriptorSet>& descriptorSets)
	{
		std::vector<vk::WriteDescriptorSet> writes = std::vector<vk::WriteDescriptorSet>(descriptorSets.size());
		for (size_t i = 0; i < writes.size(); i++)
		{
			writes[i] = vk::WriteDescriptorSet(descriptorSets[i], binding, 0, 1, descriptorType, &imageInfo[i]);
		}
		device.updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
	}

	void prepareMaterialDescriptors(SbVulkanBase& base, DescriptorAllocator& allocator, vk::DescriptorSetLayout layout)
	{
		allocateMaterialDescriptors(allocator, layout, materials.name.size(), materials.descriptorSets);
				
		writeMaterialDescriptors(base.getDevice(), 0, materials.diffuse.descriptorInfo,
			vk::DescriptorType::eCombinedImageSampler, materials.descriptorSets);
		writeMaterialDescriptors(base.getDevice(), 1, materials.specular.descriptorInfo,
			vk::DescriptorType::eCombinedImageSampler, materials.descriptorSets);
		writeMaterialDescriptors(base.getDevice(), 2, materials.bump.descriptorInfo,
			vk::DescriptorType::eCombinedImageSampler, materials.descriptorSets);
	};
};