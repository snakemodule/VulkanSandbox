#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "HelloTriangleApplication.h"

#include "Sponza.h"

int main() {

	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

