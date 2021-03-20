#pragma once

#include <fstream>
#include <vector>
#include <string>

std::vector<uint32_t> loadSpirvBinary(std::string path) 
{
	std::vector<uint32_t> binary;
	std::ifstream fileStream = std::ifstream(path, std::ios::binary);
		
	while (!fileStream.eof())
	{
		uint32_t word;
		fileStream.read(reinterpret_cast<char*>(&word), sizeof(word));
		binary.push_back(word);
	}

	fileStream.close();
	return binary;
}