#pragma once
#include <vector>
#include <math.h>

#include "Skeleton.h"
#include "glm/gtx/compatibility.hpp"
//#include "Util.h"
#include "SkeletonAnimation.h"

template <class _anim>
struct AnimationLayer
{
	std::vector<_anim> blendAnimations;
	std::vector<float> jointOpacity = std::vector<float>(0);
	glm::quat* blendedRotation = nullptr;
	glm::vec3* blendedPosition = nullptr;
};


template <class _anim>
class SkeletalAnimationComponent
{
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

public:
	//[layer][blending animations]
	std::vector<AnimationLayer<_anim>> layers;
	Skeleton * skeleton = nullptr;
	uint8_t jointCount = 0;

	double timeCounter = 0;

	//std::vector<uint8_t> parentIndex;
	
	void init(Skeleton * skeleton, std::vector<AnimationLayer<_anim>>&& layers)
	{
		this->skeleton = skeleton;
		this->jointCount = (uint8_t)skeleton->offsetMatrix.size();
		this->transformations.resize(jointCount);
		this->layers = std::move(layers);
	}

	void evaluate(float deltaTime, std::vector<float> blendFactors) {
		timeCounter += deltaTime;

		auto layerCount = layers.size();
		for (size_t i = 0; i < layerCount; i++)
		{
			for (size_t j = 0; j < layers[i].blendAnimations.size(); j++)
			{
				layers[i].blendAnimations[j].updateEvaluators(deltaTime);//todo animations should be advanced elsewhere, in batches
			}
		}

		for (size_t i = 0; i < layerCount; i++)
		{
			auto & layer = layers[i];
			auto  animationCount = layers[i].blendAnimations.size();
			if (animationCount > 1)
			{
				float span = 1.0f / (animationCount - 1);
				int index;
				auto & factor = blendFactors[i];
				if (factor < 1.0f)
					index = std::floor(factor / span);
				else
					index = (animationCount - 1);
				float remainder = std::fmod(factor, span);

				auto & blendAnim0 = layer.blendAnimations[index];
				auto & blendAnim1 = layer.blendAnimations[index + 1];
				blendAnim0.evaluate(*skeleton, timeCounter);
				blendAnim1.evaluate(*skeleton, timeCounter);
				auto & lr0 = blendAnim0.localRotation;
				auto & lr1 = blendAnim1.localRotation;
				auto & lp0 = blendAnim0.localPosition;
				auto & lp1 = blendAnim1.localPosition;
				for (size_t j = 0; j < jointCount; j++)
					lr0[j] = glm::slerp(lr0[j], lr1[j], remainder / span);
				for (size_t j = 0; j < jointCount; j++)
					lp0[j] = glm::lerp(lp0[j], lp1[j], remainder / span);
				layer.blendedRotation = lr0.data();
				layer.blendedPosition = lp0.data();
			}
			else
			{
				layer.blendAnimations[0].evaluate( *skeleton, timeCounter);
				layer.blendedRotation = layer.blendAnimations[0].localRotation.data();
				layer.blendedPosition = layer.blendAnimations[0].localPosition.data();
			}

		}
		//blend all layers into base layer
		auto & accumulatorRotations = layers[0].blendedRotation;
		auto & accumulatorPositions = layers[0].blendedPosition;
		for (size_t i = 1; i < layerCount; i++)
			for (size_t joint = 0; joint < jointCount; joint++) {
				auto & currentLayerRotations = layers[i].blendedRotation;
				accumulatorRotations[joint] = glm::slerp(accumulatorRotations[joint], currentLayerRotations[joint], layers[i].jointOpacity[joint]);
			}

		for (size_t i = 1; i < layerCount; i++)
			for (size_t joint = 0; joint < jointCount; joint++)
			{
				auto & currentLayerPositions = layers[i].blendedPosition;
				accumulatorPositions[joint] = glm::lerp(accumulatorPositions[joint], currentLayerPositions[joint], layers[i].jointOpacity[joint]);
			}
		//base layer -> local transformation matrices
		for (size_t i = 0; i < jointCount; i++)
			transformations[i] = glm::translate(glm::mat4(1.0f), accumulatorPositions[i]) * glm::toMat4(accumulatorRotations[i]);
		//local matrices -> global matrices
		for (size_t i = 1; i < jointCount; i++)
			transformations[i] = transformations[skeleton->hierarchy[i]] * transformations[i];
		//apply inverse bind pose
		for (size_t i = 0; i < jointCount; i++)
			transformations[i] = transformations[i] * skeleton->offsetMatrix[i];
	}



	std::vector<glm::mat4> transformations;
};

