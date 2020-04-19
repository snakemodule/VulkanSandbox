#pragma once

#include <vector>

class UncompressedAnimationKeys
{
public:
	double animationDuration;
	size_t rotKeyCount;
	size_t posKeyCount;

	//todo include from other project?
	struct qKey
	{
		char channel_id;
		unsigned int time_id;
		double time;
		unsigned int type;
		double w;
		double x;
		double y;
		double z;
		size_t animationIndex;
	};

	struct vKey
	{
		char channel_id;
		unsigned int time_id;
		double time;
		unsigned int type;
		float x;
		float y;
		float z;
		size_t animationIndex;
	};

	std::vector<qKey> rotData;
	std::vector<vKey> posData;

	void loadAnimationData(std::string filename);
	 
	

	UncompressedAnimationKeys();
	~UncompressedAnimationKeys();
};

