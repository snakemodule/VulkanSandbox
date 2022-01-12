#pragma once

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags


#include "SceneStuff.h"

class Sponza
{
public:
	

	static void load(SbVulkanBase * base) {

		Assimp::Importer modelImporter;
		const aiScene* modelScene;

		//modelScene = modelImporter.ReadFile("models/Willems/sponza.dae",//correct?
		//	aiProcess_CalcTangentSpace |
		//	aiProcess_Triangulate |
		//	aiProcess_JoinIdenticalVertices |
		//	aiProcess_SortByPType |
		//	aiProcess_ValidateDataStructure);

		Scene scene = Scene();

		scene.load("models/Willems/sponza.dae", base);

	}


};

