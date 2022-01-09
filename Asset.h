#pragma once

#include "SbDescriptorSet.h"

//middle man between shader and resources
class Asset
{
public:
	//outside of asset
	//VP MATRIX
	//descriptorset global
	 
	//________________

	//Mesh

	//unique to the asset
	//_diffuse = nullptr
	//_bump = nullptr
	//_specular = nullptr
	//_displacement = nullptr

	



	//animation(s)? separate asset?
	//skeleton definition
	
	//Shader/subpass/pipeline subpass ID
	//shader reflection automatically gets the textures and complains if they are null
	//descriptorset layout for the asset

	SbShaderLayout* layout;
	SbDescriptorSet* set;




	//__________________
	
	//this is per instance of the asset
	//skeleton transforms uniform buffer
	//model matrix
	//descriptorset for the asset instance

};

/*
set 0 glbal
set 1 asset
set 2 instance
*/