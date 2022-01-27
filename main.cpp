#include "HelloTriangleApplication.h"
#include "VkTest.h"



//#include "Sponza.h"

int main() {

	//HelloTriangleApplication app;
	VkTest app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

