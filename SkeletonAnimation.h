#pragma once

#include <assimp/scene.h>
#include <vector>
#include <array>
#include "spline.h"

#include "AnimationKeys.h"

//#include "boost/multi_array.hpp"

#include "Skeleton.h"

#include <memory>

class SkeletonAnimation
{
private:
	struct plotV {
		std::vector<float> x;
		std::vector<float> y;
		std::vector<float> z;
		float* p_x;
		float* p_y;
		float* p_z;
		int L = 0;

		void update(float x, float y, float z)
		{
			this->x.push_back(x);
			this->y.push_back(y);
			this->z.push_back(z);
			p_x = this->x.data();
			p_y = this->y.data();
			p_z = this->z.data();
			L++;
		}

		void clear() {
			x.clear();
			y.clear();
			z.clear();
		}
	} livePlot, liveSamp;
	int L = 0;


	struct quatKey {
		glm::quat rot;
		float time;
		int channel;

	};
	struct vecKey {
		glm::vec3 pos;
		float time;
		int channel;
	};

	struct uint8_ring4 {
		uint8_t n;

		uint8_ring4& operator++() {
			n = (n + 1) % 4;
			return *this;
		};

		uint8_t operator+(uint8_t rhs) {
			return (n + rhs) % 4;
		}
	};

	std::vector<uint8_ring4> rotKeyRingIndex;
	std::vector<uint8_ring4> posKeyRingIndex;
	std::vector<std::array<quatKey, 4>> rotKeys;
	std::vector<std::array<vecKey, 4>> posKeys;
	std::vector<QuaternionInterpolation> rotInterpolators;
	std::vector<Vector3Interpolation> posInterpolators;

	SkeletonAnimation::quatKey bitsToKey(AnimationKeys::qbits bits);
	float dequantizeTime20bits(unsigned int bits);
	SkeletonAnimation::vecKey bitsToKey(AnimationKeys::vbits bits);

	AnimationKeys::qbits getRotData();
	//AnimationKeys::qbits getNextRotData(size_t channel);
	AnimationKeys::qbits getNextRotData();

	AnimationKeys::vbits getPosData();
	AnimationKeys::vbits getNextPosData();

	AnimationKeys::qbits latestRotData;
	AnimationKeys::vbits latestPosData;
	uint32_t rotDataIndex = 0;
	uint32_t posDataIndex = 0;
	std::shared_ptr<AnimationKeys> animationData;


public:
	

	void plot(Skeleton s);


	void evaluate();
	
	void updateEvaluators(float animationTime, float playbackMultiplier = 1);

	void initiateEvaluators();

	SkeletonAnimation(int jointCount, std::shared_ptr<AnimationKeys> animationData);
	~SkeletonAnimation();

	double animationTime = 0;
	
	double getAnimationDuration();

	std::vector<glm::quat> localRotation;
	std::vector<glm::vec3> localPosition;



	


	
};

