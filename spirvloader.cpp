#include "spirvloader.h"

std::vector<uint32_t> loadSpirvBinary(std::string path)
{
	std::vector<uint32_t> binary;
	std::ifstream fileStream = std::ifstream(path, std::ios::binary);

	while (!fileStream.eof() && fileStream.is_open())
	{
		uint32_t word;
		fileStream.read(reinterpret_cast<char*>(&word), sizeof(word));
		binary.push_back(word);
	}

	fileStream.close();
	return binary;
}
