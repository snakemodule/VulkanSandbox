#include "Model.h"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "AnimationStuff.h"

#include "ResourceManager.h"

AnimatedModel::AnimatedModel(std::string modelFile)
{
	Assimp::Importer modelImporter;
	const aiScene* modelScene;

	modelScene = modelImporter.ReadFile(modelFile.c_str(),//"jump.fbx",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_ValidateDataStructure);

	std::map<std::string, uint64_t> jointLayout;
	auto rootJoint = AnimationStuff::findRootJoint(modelScene->mRootNode, modelScene);

	AnimationStuff::makeFlatSkeleton(rootJoint, skeleton, jointLayout, 0, -1, modelScene);
	AnimationStuff::finalizeFlatten(skeleton);

	//hardcoded animation load
	auto running = ResourceManager::getInstance().loadAnimation("running.chs");//running.loadAnimationData("running.chs");
	auto walking = ResourceManager::getInstance().loadAnimation("walking.chs");//walking.loadAnimationData("walking.chs");
	AnimationLayer baseLayer;
	baseLayer.blendAnimations.emplace_back(skeleton.jointCount, walking);
	baseLayer.blendAnimations.emplace_back(skeleton.jointCount, running);
	skeletalAnimationComponent.init(&skeleton,
		std::vector<AnimationLayer>{ baseLayer });

	meshes.resize(modelScene->mNumMeshes);
	for (unsigned int i = 0; i < modelScene->mNumMeshes; i++) {
		const aiMesh* currentMesh = modelScene->mMeshes[i];

		//read vertex data
		for (size_t j = 0; j < currentMesh->mNumVertices; j++)
		{
			glm::vec3 pos = {
				currentMesh->mVertices[j].x,
				currentMesh->mVertices[j].y,
				currentMesh->mVertices[j].z
			};

			glm::vec2 texCoords = { 0.0f, 0.0f };
			if (currentMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				texCoords.x = currentMesh->mTextureCoords[0][j].x;
				texCoords.y = currentMesh->mTextureCoords[0][j].y;
			}

			glm::vec3 color = { 1.0f, 1.0f, 1.0f };

			glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
			normal.x = currentMesh->mNormals[j].x;
			normal.y = currentMesh->mNormals[j].y;
			normal.z = currentMesh->mNormals[j].z;

			meshes[i].vertexBuffer.emplace_back(
				pos,
				color,
				texCoords,
				normal
			);
		}

		//get vertex indices
		for (size_t j = 0; j < modelScene->mMeshes[i]->mNumFaces; j++)
		{
			for (size_t k = 0; k < modelScene->mMeshes[i]->mFaces[j].mNumIndices; k++)
			{
				meshes[i].indexBuffer.push_back(modelScene->mMeshes[i]->mFaces[j].mIndices[k]);
			}
		}

		//add weights and bone indices to vertices
		for (size_t j = 0; j < currentMesh->mNumBones; j++)
		{
			size_t boneHierarchyIndex = jointLayout[currentMesh->mBones[j]->mName.C_Str()];
			for (size_t k = 0; k < currentMesh->mBones[j]->mNumWeights; k++) {
				size_t meshVertexID = currentMesh->mBones[j]->mWeights[k].mVertexId;
				float weight = currentMesh->mBones[j]->mWeights[k].mWeight;
				meshes[i].vertexBuffer[meshVertexID].addBoneData(boneHierarchyIndex, weight);
			}
		}

		if (modelScene->HasMaterials())
		{
			auto material = modelScene->mMaterials[currentMesh->mMaterialIndex];

			aiColor4D specular;
			aiColor4D diffuse;
			aiColor4D ambient;
			float shine;

			aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
			aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shine);

			meshes[i].material.diffuseColor = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
			meshes[i].material.ambientColor = { ambient.r, ambient.g , ambient.b ,ambient.a };
			meshes[i].material.specularColor = { specular.r, specular.g, specular.b, specular.a };
		}
	}
}
