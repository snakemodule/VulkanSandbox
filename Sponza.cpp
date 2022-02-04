#include "Sponza.h"

void Sponza::load(SbVulkanBase* base) {

	//Assimp::Importer modelImporter;
	//const aiScene* modelScene;

	//modelScene = modelImporter.ReadFile("models/Willems/sponza.dae",//correct?
	//	aiProcess_CalcTangentSpace |
	//	aiProcess_Triangulate |
	//	aiProcess_JoinIdenticalVertices |
	//	aiProcess_SortByPType |
	//	aiProcess_ValidateDataStructure);

	scene.load("models/Willems/sponza.dae", base);
}
