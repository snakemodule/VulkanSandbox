#include "SbCamera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 SbCamera::getViewMatrix()
{
	glm::mat4 X = { 
		glm::vec4(1, 0, 0, 0), 
		glm::vec4(0, -1, 0, 0), 
		glm::vec4(0, 0, -1, 0), 
		glm::vec4(0, 0, 0, 1) 
	};
	X = glm::inverse(X);
	
	return  glm::lookAt(position, forward+position, globalUp);
}

glm::mat4 SbCamera::getProjectionMatrix()
{
	auto proj = glm::perspective(glm::radians(85.0f), width / (float)height, zNear, zFar);
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
