#pragma once

#include <assimp/scene.h>
#include <vector>
#include <array>
#include "spline.h"

#include "UncompressedAnimationKeys.h"

#include "Skeleton.h"

class UncompressedAnimation
{
private:
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


	UncompressedAnimation::quatKey getRotData();
	UncompressedAnimation::quatKey getNextRotData();

	UncompressedAnimation::vecKey getPosData();
	UncompressedAnimation::vecKey getNextPosData();

	UncompressedAnimation::quatKey latestRotData;
	UncompressedAnimation::vecKey latestPosData;
	uint32_t rotDataIndex = 0;
	uint32_t posDataIndex = 0;
	UncompressedAnimationKeys & animationData;


public:


	void plot(Skeleton s);

	//void evaluate();

	void evaluate(Skeleton s, double t);

	void updateEvaluators(float deltaTime);

	void initiateEvaluators();

	float animationTime = 0;


	std::vector<glm::quat> localRotation;
	std::vector<glm::vec3> localPosition;

	UncompressedAnimation(int jointCount, UncompressedAnimationKeys & animationData);
	~UncompressedAnimation();
};

