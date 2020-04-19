#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <assimp/scene.h> 

struct Skeleton {
	uint8_t jointCount = 0;
	std::vector<uint8_t> hierarchy;
	std::vector<glm::mat4> offsetMatrix;
	std::vector<aiNodeAnim*> assimpAnimChannel;
};