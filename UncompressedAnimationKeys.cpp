#include "UncompressedAnimationKeys.h"

#include <fstream>

void UncompressedAnimationKeys::loadAnimationData(std::string filename)
{
	std::ifstream file;
	file.open(filename, std::ios::binary);
	if (file.is_open())
	{
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(&animationDuration), sizeof(animationDuration));
		file.read(reinterpret_cast<char*>(&rotKeyCount), sizeof(rotKeyCount));
		file.read(reinterpret_cast<char*>(&posKeyCount), sizeof(posKeyCount));
		rotData.resize(rotKeyCount); //each key is 3*uint32 big
		posData.resize(posKeyCount); //each key is 3*uint32 big

		file.read(reinterpret_cast<char*>(rotData.data()), sizeof(qKey) * rotData.size());
		file.read(reinterpret_cast<char*>(posData.data()), sizeof(vKey) * posData.size());
	}
}

UncompressedAnimationKeys::UncompressedAnimationKeys()
{
}


UncompressedAnimationKeys::~UncompressedAnimationKeys()
{
}
