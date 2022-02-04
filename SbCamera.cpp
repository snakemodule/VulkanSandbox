#include "SbCamera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 SbCamera::getViewMatrix()
{
	return glm::lookAt(position, facing, globalUp);
}

glm::mat4 SbCamera::getProjectionMatrix()
{
	auto proj = glm::perspective(glm::radians(85.0f), width / (float)height, 0.1f, 10.0f);
	proj[1][1] *= -1;
	return proj;
}

SbCamera::SbCamera(int width, int height)
{
	this->width = width;
	this->height = height;
}


SbCamera::~SbCamera()
{
}
