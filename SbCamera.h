#pragma once

#include <glm/glm.hpp>

class SbCamera
{
public:

	glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

	const float zNear = 0.1f;
	const float zFar = 10.f;

	uint32_t width;
	uint32_t height;


	glm::mat4 getViewMatrix();

	glm::mat4 getProjectionMatrix();

	

	SbCamera(int width, int height);
	~SbCamera();
};

