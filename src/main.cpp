#include <iostream>

int main(int /*argc*/, char** /*argv*/) try {
	std::cout << "Hello, World!" << srd::endl;
	return 0;
} catch (std::exception& e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Unknown error" << std::endl;
	return 1;
}
