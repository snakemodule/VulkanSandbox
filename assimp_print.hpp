#pragma once

#include <iostream>

#include <assimp/scene.h> // Output data structure

namespace assimp_loading {
	void indent(int indent) {
		for (size_t i = 0; i < indent; i++)
		{
			std::cout << "|  ";
		}
	}

	void PrintNode(const aiNode* node, const aiScene* scene, int nrindent) {
		indent(nrindent);
		std::cout << "name: " << node->mName.C_Str() << "\n";
		indent(nrindent);
		std::cout << "numchil: " << node->mNumChildren << "\n";
		indent(nrindent);
		std::cout << "nummesh: " << node->mNumMeshes << "\n";

		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			indent(nrindent);
			std::cout << "mesh index: " << node->mMeshes[i] << "\n";
			indent(nrindent);
			std::cout << "number of bones: " << scene->mMeshes[i]->mNumBones << "\n";
			for (size_t j = 0; j < scene->mMeshes[i]->mNumBones; j++)
			{
				indent(nrindent);
				std::cout << "mesh bones: " << scene->mMeshes[i]->mBones[j]->mName.C_Str() << "\n";

				indent(nrindent);
				std::cout << "vertexes affected: " << scene->mMeshes[i]->mBones[j]->mNumWeights << "\n";
			}
		}



		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			PrintNode(node->mChildren[i], scene, nrindent + 1);
		}
	}

}