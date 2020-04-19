#pragma once

#include <string>
#include <fstream>
#include <vector>

class AnimationKeys
{
public:
	double animationDuration;
	size_t rotKeyCount;
	size_t posKeyCount;

	//todo include from other project?
	struct qbits {
		unsigned int type : 2;
		unsigned int channel : 7;
		unsigned int time : 20;
		unsigned int c0sign : 1;
		unsigned int c1sign : 1;
		unsigned int c2sign : 1;
		//--32
		unsigned int nullc : 2;
		unsigned int x : 20;
		unsigned int y0 : 10;
		//--32
		unsigned int reserved : 1;
		unsigned int nullsign : 1;
		unsigned int y1 : 10;
		unsigned int z : 20;
		//--32
	};

	struct vbits {
		unsigned int type : 2;
		unsigned int channel : 7;
		unsigned int time : 20;
		unsigned int c0sign : 1;
		unsigned int c1sign : 1;
		unsigned int c2sign : 1;
		//--32
		unsigned int reserved : 1;
		unsigned int x : 21;
		unsigned int y0 : 10;
		//--32
		unsigned int y1 : 11;
		unsigned int z : 21;
		//--32
	};

	std::vector<qbits> rotData;
	std::vector<vbits> posData;

	void loadAnimationData(std::string filename);

	

	AnimationKeys();
	~AnimationKeys();
};

