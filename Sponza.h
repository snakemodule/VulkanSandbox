#pragma once



#include "SceneStuff.h"

class Sponza
{
public:
	Scene scene;

	void load(SbVulkanBase & base) {
		scene.load("models/Willems/sponza.dae", &base);
	}


};

